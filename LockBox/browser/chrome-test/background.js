console.log("LockBox background loaded");

// 🔐 Click extension icon
chrome.action.onClicked.addListener(async (tab) => {

  if (!tab || !tab.id || !tab.url) {
    console.log("No active tab");
    return;
  }

  console.log("Extension clicked for:", tab.url);

  // Inject content script
  try {
    await chrome.scripting.executeScript({
      target: { tabId: tab.id },
      files: ["content.js"]
    });
  } catch {
    console.log("content.js already injected");
  }

  // Load stored pairing data
  chrome.storage.local.get(["client_id", "client_key"], (data) => {

    // 🔴 FIRST TIME → PAIR
    if (!data.client_id || !data.client_key) {

      console.log("No pairing found. Starting pairing...");

      const ws = new WebSocket("ws://127.0.0.1:19455");

      ws.onopen = () => {
        ws.send(JSON.stringify({ action: "pair" }));
      };

      ws.onmessage = (e) => {
        const res = JSON.parse(e.data);

        if (!res.success) {
          console.log("Pairing rejected");
          return;
        }

        console.log("Pairing success");

        chrome.storage.local.set({
          client_id: res.client_id,
          client_key: res.key
        });

        console.log("Stored pairing. Click extension again.");
      };

      return;
    }

    // 🟢 ALREADY PAIRED → SECURE AUTOFILL
    const CLIENT_ID = data.client_id;
    const CLIENT_KEY = data.client_key;

    const ws = new WebSocket("ws://127.0.0.1:19455");

    ws.onopen = async () => {

      console.log("Connected to LockBox (secure)");

      const key = Uint8Array.from(atob(CLIENT_KEY), c => c.charCodeAt(0));
      const nonce = crypto.getRandomValues(new Uint8Array(12));

      const payload = new TextEncoder().encode(JSON.stringify({
        client_id: CLIENT_ID,
        action: "getCredentials",
        url: tab.url
      }));

      const cryptoKey = await crypto.subtle.importKey(
        "raw",
        key,
        "AES-GCM",
        false,
        ["encrypt"]
      );

      const ciphertext = await crypto.subtle.encrypt(
        { name: "AES-GCM", iv: nonce },
        cryptoKey,
        payload
      );

      ws.send(JSON.stringify({
        client_id: CLIENT_ID,
        nonce: btoa(String.fromCharCode(...nonce)),
        ciphertext: btoa(String.fromCharCode(...new Uint8Array(ciphertext)))
      }));
    };

    ws.onmessage = async (event) => {

      console.log("Encrypted response from LockBox:", event.data);

      const msg = JSON.parse(event.data);

      if (!msg.ciphertext) return;

      const key = Uint8Array.from(atob(CLIENT_KEY), c => c.charCodeAt(0));
      const nonce = Uint8Array.from(atob(msg.nonce), c => c.charCodeAt(0));
      const ciphertext = Uint8Array.from(atob(msg.ciphertext), c => c.charCodeAt(0));

      const cryptoKey = await crypto.subtle.importKey(
        "raw",
        key,
        "AES-GCM",
        false,
        ["decrypt"]
      );

      const decrypted = await crypto.subtle.decrypt(
        { name: "AES-GCM", iv: nonce },
        cryptoKey,
        ciphertext
      );

      const res = JSON.parse(new TextDecoder().decode(decrypted));

      if (!res.success) {
        console.log("LockBox error:", res.error);
        return;
      }

      const entries = res.payload.entries || [];
      if (!entries.length) {
        console.log("No credentials found");
        return;
      }

      const entry = entries[0];

      chrome.tabs.sendMessage(tab.id, {
        type: "lockboxFill",
        entry: entry
      });
    };
  });
});