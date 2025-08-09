// server.js
const http = require("node:http");
const { WebSocketServer } = require("ws");

// If you're on Node 18+, global fetch exists. If not:
const fetch = (...args) => import('node-fetch').then(({default: f}) => f(...args));

const PORT = 5555;

// ---- In-memory state -------------------------------------------------
const names = new Map();        // ws -> "Jeffrey"
const prefs = new Map();        // ws -> { lang: "en" }
const langSet = new Set();      // track which langs are in use (for caching)
const cache = new Map();        // key `${text}::${toLang}` -> translated text

// ---- Translation config (swap this out for your provider) ------------
const LT_ENDPOINT = process.env.LT_ENDPOINT || "https://libretranslate.com/translate";
// If you self-host LibreTranslate, point LT_ENDPOINT to your instance.
// For Google/DeepL, change translate() accordingly.
const LT_API_KEY = process.env.LT_API_KEY || ""; // optional

// async function translate(text, toLang) {
//   if (!text || !toLang) return text;
//   // No-op if the target lang is English and text is likely English? Keep it simple:
//   const cacheKey = `${text}::${toLang}`;
//   if (cache.has(cacheKey)) return cache.get(cacheKey);

//   const body = {
//     q: text,
//     source: "auto",
//     target: toLang,
//     format: "text",
//   };
//   if (LT_API_KEY) body.api_key = LT_API_KEY;

//   try {
//     const res = await fetch(LT_ENDPOINT, {
//       method: "POST",
//       headers: { "Content-Type": "application/json" },
//       body: JSON.stringify(body),
//     });
//     const data = await res.json();
//     const out = data?.translatedText ?? text;
//     cache.set(cacheKey, out);
//     return out;
//   } catch (err) {
//     console.error("translate error:", err);
//     return text; // fail open
//   }
// }

// async function translate(text, toLang) {
//   // Visual stub to prove routing works:
//   if (toLang === 'zh') return `【中译】${text}`;
//   if (toLang === 'es') return `【ES】${text}`;
//   return text; // en or anything else
// }

async function translate(text, toLang) {
  const cacheKey = `${text}::${toLang}`;
  if (cache.has(cacheKey)) return cache.get(cacheKey);

  try {
    const res = await fetch("https://libretranslate.com/translate", {
      method: "POST",
      headers: { "Content-Type": "application/json", "Accept": "application/json" },
      body: JSON.stringify({ q: text, source: "auto", target: toLang, format: "text" })
    });
    if (!res.ok) {
      console.error('[translate] HTTP', res.status, await res.text());
      return text; // fail open
    }
    const data = await res.json();
    const out = data?.translatedText ?? text;
    cache.set(cacheKey, out);
    return out;
  } catch (e) {
    console.error('[translate] error', e);
    return text;
  }
}



// ---- Helpers ---------------------------------------------------------
function send(ws, text) {
  if (ws.readyState === ws.OPEN) ws.send(text);
}

function broadcastRaw(text) {
  // Fallback for plain strings (system lines)
  for (const client of wss.clients) if (client.readyState === client.OPEN) client.send(text);
  console.log("Broadcast:", text);
}

async function broadcastTranslated(fromName, text) {
  // Collect target languages in use (at send time)
  const targetsByLang = new Map(); // lang -> [ws]
  for (const ws of wss.clients) {
    if (ws.readyState !== ws.OPEN) continue;
    const lang = prefs.get(ws)?.lang || "en";
    if (!targetsByLang.has(lang)) targetsByLang.set(lang, []);
    targetsByLang.get(lang).push(ws);
  }

  // Translate once per language, then fan out
  for (const [lang, sockets] of targetsByLang.entries()) {
    const translated = await translate(`${fromName}: ${text}`, lang);
    const payload = translated; // keeping it simple as plain text
    for (const ws of sockets) send(ws, payload);
  }
}

// ---- Server & WebSocket ----------------------------------------------
const server = http.createServer((_req, res) => {
  res.writeHead(200, { "Content-Type": "text/plain" });
  res.end("OK\n");
});

const wss = new WebSocketServer({ server });

wss.on("connection", (ws) => {
  // defaults
  prefs.set(ws, { lang: "en" });
  send(ws, "Welcome! Send your name (first message). You can change language via JSON: {\"type\":\"setLang\",\"lang\":\"zh\"}");

  let userName = ""; // first non-JSON message becomes the name

  ws.on("message", async (data) => {
    const raw = data.toString().trim();

    // Try to parse JSON; if fails, treat as plain text
    let parsed = null;
    if (raw.startsWith("{")) {
      try { parsed = JSON.parse(raw); } catch {}
    }

    // 1) Name setup (first plain text)
    if (!userName && !parsed) {
      userName = raw || "Anonymous";
      names.set(ws, userName);
      broadcastRaw(`🔵 ${userName} joined`);
      return;
    }

    // 2) Commands / JSON protocol
    if (parsed) {
      if (parsed.type === "setLang" && typeof parsed.lang === "string") {
        const lang = parsed.lang.trim().toLowerCase();
        prefs.set(ws, { lang });
        langSet.add(lang);
        send(ws, `✅ Language set to "${lang}"`);
        return;
      }
      if (parsed.type === "chat" && typeof parsed.text === "string") {
        if (!userName) { send(ws, "Please send your name first."); return; }
        await broadcastTranslated(userName, parsed.text);
        return;
      }
      // Unknown JSON: ignore
      return;
    }

    // 3) Plain commands
    if (raw === "/quit") {
      ws.close();
      return;
    }

    // 4) Plain text as chat (backward compatible)
    if (!userName) {
      userName = raw || "Anonymous";
      names.set(ws, userName);
      broadcastRaw(`🔵 ${userName} joined`);
      return;
    }
    await broadcastTranslated(userName, raw);
  });

  ws.on("close", () => {
    if (userName) broadcastRaw(`🔴 ${userName} left`);
    names.delete(ws);
    prefs.delete(ws);
  });

  ws.on("error", (e) => console.error("WS error:", e));
});

server.listen(PORT, "0.0.0.0", () => {
  console.log(`Listening on http://localhost:${PORT} (ws://localhost:${PORT})`);
});
