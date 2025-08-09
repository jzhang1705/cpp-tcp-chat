# WebSocket + C++ TCP Chat Project

This repository contains **two versions** of a real-time chat application:

1. **JavaScript/WebSocket version** (active, scalable, web-based UI)
2. **C++ TCP socket version** (archived in `NotUsedCppCode/`, console-based)

The WebSocket version is now the main focus, but the original C++ code remains for reference.

---

## 📂 Project Structure
```

.
├── public/                # Static frontend (HTML, CSS, JS)
├── server.js               # WebSocket server (Node.js)
├── package.json            # Node.js project configuration
├── NotUsedCppCode/         # Archived C++ TCP server/client
│   ├── server.cpp
│   ├── client.cpp
│   └── Makefile
└── README.md

````

---

## 🖥 JavaScript / WebSocket Application

### Overview
The JS version uses:
- **Node.js** with the [`ws`](https://www.npmjs.com/package/ws) library for the WebSocket server
- Static HTML/CSS/JS frontend for a browser-based chat UI
- Broadcasts messages to all connected clients, including the sender
- Simple and easy to scale with cloud hosting (Heroku, Render, AWS, etc.)

### Run Locally
**1. Install dependencies**
```bash
npm install
````

**2. Start the WebSocket server**

```bash
npm run dev
```

The server will start on **`ws://localhost:5555`**.

**3. Open the client**
Open `public/index.html` in your browser (double-click or use a static server).

---

## 💾 C++ TCP Socket Application (Archived)

### Overview

The C++ version implements:

* A **TCP server** using POSIX sockets (`<sys/socket.h>`, `<arpa/inet.h>`)
* A **TCP client** for terminal-based chat
* Multi-client support with `std::thread` and `std::mutex`
* Basic message broadcasting to all connected clients

This version was the starting point for learning low-level networking concepts, but is **not actively maintained** in favor of the WebSocket version.

### Build & Run

From inside `NotUsedCppCode/`:

**1. Build**

```bash
make
```

**2. Run server**

```bash
./server
```

**3. Run client (in another terminal)**

```bash
./client
```

**4. Connect multiple clients** to see broadcast behavior.

---

## 🌟 Why Two Versions?

* **C++ version**: Teaches fundamentals of TCP/IP, manual socket handling, and threading.
* **JavaScript/WebSocket version**: Web-friendly, easier to scale, and accessible via any browser.

---

## 🚀 Next Steps

* Add **usernames** and styling to the WebSocket UI
* Deploy WebSocket server to a cloud provider
* Optionally integrate with a database for message history
* Experiment with translation features (e.g., English ↔ Chinese)

---

## 📜 License

MIT License – feel free to modify and use for your own projects.

```
