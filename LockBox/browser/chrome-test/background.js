console.log("LockBox extension background loaded");

function getCredentialsForTab(tab) {
  if (!tab || !tab.id || !tab.url) {
    console.log("No active tab or URL");
    return;
  }

  const url = tab.url;
  console.log("Requesting credentials for:", url);

  chrome.runtime.sendNativeMessage(
    "com.lockbox.nativehost",
    { 
      version: 1,
      action: "getCredentials",
      requestId: "tab-" + tab.id + "-" + Date.now(),
      payload: { url }
    },
    (response) => {
      if (chrome.runtime.lastError) {
        console.error("Native message error:", chrome.runtime.lastError.message);
        return;
      }
      console.log("Received from LockBox:", response);

      if (!response || !response.success || !response.payload) {
        console.log("No success / payload in response");
        return;
      }

      const entries = response.payload.entries || [];
      if (!entries.length) {
        console.log("No credentials found for this site");
        return;
      }

      const entry = entries[0];

      chrome.tabs.sendMessage(tab.id, {
        type: "lockboxFill",
        entry
      });
    }
  );
}

// When user clicks the extension icon → try autofill
chrome.action.onClicked.addListener((tab) => {
  getCredentialsForTab(tab);
});
