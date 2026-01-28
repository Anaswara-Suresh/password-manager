console.log("LockBox content script loaded");

/**
 * Find username + password fields using heuristics
 */
function findLoginFields() {
  if (document.visibilityState !== "visible") return null;

  const passwordFields = Array.from(
    document.querySelectorAll('input[type="password"]')
  );

  if (!passwordFields.length) return null;

  const passwordField = passwordFields[0];

  // Find candidate username fields in same form
  const form = passwordField.form || document;
  const candidates = Array.from(
    form.querySelectorAll(
      'input[type="text"], input[type="email"], input:not([type])'
    )
  );

  let usernameField = null;

  if (candidates.length === 1) {
    usernameField = candidates[0];
  } else {
    // Heuristic: nearest field ABOVE password
    const pwdRect = passwordField.getBoundingClientRect();
    let best = null;
    let bestDist = Infinity;

    for (const c of candidates) {
      const r = c.getBoundingClientRect();
      const dy = pwdRect.top - r.top;
      if (dy > 0 && dy < bestDist) {
        bestDist = dy;
        best = c;
      }
    }

    usernameField = best || candidates[0] || null;
  }

  return { usernameField, passwordField };
}

/**
 * Try filling fields, retrying for dynamic pages
 */
function tryFill(entry, retries = 3) {
  const fields = findLoginFields();

  if (!fields) {
    if (retries > 0) {
      setTimeout(() => tryFill(entry, retries - 1), 300);
    }
    return;
  }

  const { usernameField, passwordField } = fields;
  const { username, password } = entry;

  // Fill username if empty
  if (usernameField && !usernameField.value) {
    usernameField.focus();
    usernameField.value = username;
    usernameField.dispatchEvent(new Event("input", { bubbles: true }));
    usernameField.dispatchEvent(new Event("change", { bubbles: true }));
  }

  // Fill password if empty
  if (passwordField && !passwordField.value) {
    passwordField.focus();
    passwordField.value = password;
    passwordField.dispatchEvent(new Event("input", { bubbles: true }));
    passwordField.dispatchEvent(new Event("change", { bubbles: true }));
  }

  console.log("LockBox autofill done");
}

/**
 * Listen for background messages
 */
chrome.runtime.onMessage.addListener((msg) => {
  if (msg?.type === "lockboxFill" && msg.entry) {
    console.log("LockBox filling:", msg.entry.username);
    tryFill(msg.entry);
  }
});
