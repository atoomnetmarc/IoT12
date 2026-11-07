const $ = (id) => document.getElementById(id);

let unlocked = false;
let portalActive = false;

function setText(id, text) {
  const el = $(id);
  if (el) {
    el.textContent = text;
  }
}

function fmt(value, digits) {
  return Number.isFinite(value) ? value.toFixed(digits) : "--";
}

// The firmware sends the state name; the long sentences live here.
// Keep this table in sync with the HandleDiagnosis texts in the firmware.
const HANDLE_TEXT = {
  OK: { title: "Handle and tip are connected", action: "Ready to heat." },
  NO_HANDLE: { title: "No handle is connected", action: "Connect the T12 handle to the station." },
  NO_TIP: { title: "The T12 tip is not seated", action: "Press the tip firmly into the handle." },
  SUPPLY_OFF: { title: "The 24 V supply is off", action: "Switch the 24 V supply on." },
  LOSS: { title: "The heat controller is not responding", action: "Check the link cable and the heat controller." },
};

function applyState(state) {
  setText("v-tip", fmt(state.tip, 1) + " C");
  setText("v-set", fmt(state.set, 1) + " C");
  setText("v-ntc", fmt(state.ntc, 1) + " C");
  setText("v-duty", fmt(state.duty, 0) + " %");
  setText("v-current", fmt(state.current, 2) + " A");
  setText("v-heater", state.heater ? "ON" : "OFF");
  setText("v-link", state.linked ? "OK" : "LOST");

  const handle = HANDLE_TEXT[state.handle] || { title: state.handle || "--", action: "" };
  setText("v-handle", handle.title);
  setText("v-handle-action", handle.action);
  const handleTile = $("v-handle-tile");
  if (handleTile) {
    handleTile.classList.toggle("ok", state.handle === "OK");
    handleTile.classList.toggle("fault", state.handle !== "OK");
  }

  setText("v-ntc-v", fmt(state.ntc_v, 3) + " V");
  setText("v-tc-mv", fmt(state.tc_mv, 1) + " mV");
  setText("v-vin-off", fmt(state.vin_off, 2) + " V");
  setText("v-vin-on", fmt(state.vin_on, 2) + " V");
  setText("v-iron-off", fmt(state.iron_off, 3) + " A");
  setText("v-iron-on", fmt(state.iron_on, 3) + " A");
  setText("v-rtip", fmt(state.rtip, 2) + " ohm");
  setText("v-rsupply", fmt(state.rsupply, 2) + " ohm");

  setText("n-state", state.wifi);
  setText("n-ssid", state.ssid || "-");
  setText("n-ip", state.ip || "-");
  setText("n-rssi", state.rssi + " dBm");
  setText("d-uptime", state.uptime + " s");
  setText("d-heap", state.heap + " B");

  const badge = $("badge");
  if (state.portal) {
    badge.textContent = "portal";
    badge.className = "badge portal";
  } else if (state.wifi === "connected") {
    badge.textContent = "online";
    badge.className = "badge online";
  } else {
    badge.textContent = state.wifi;
    badge.className = "badge offline";
  }

  portalActive = Boolean(state.portal);
  document.querySelectorAll(".station-only").forEach((el) => {
    el.classList.toggle("hidden", portalActive);
  });
  applyAccess();
}

function applyAccess() {
  const locked = !unlocked;
  $("lock-form").classList.toggle("hidden", !locked);
  $("lock-now").classList.toggle("hidden", locked);
  $("regenerate").classList.toggle("hidden", locked);
  document.querySelectorAll(".write-card").forEach((el) => {
    el.classList.toggle("hidden", !(unlocked || portalActive));
  });
}

const STREAM_RETRY_MIN_MS = 1000;
const STREAM_RETRY_MAX_MS = 15000;

let stream = null;
let streamRetryMs = STREAM_RETRY_MIN_MS;
let streamRetryTimer = null;

function setStreamStatus(text, cls) {
  const el = $("stream-status");
  if (el) {
    el.textContent = text;
    el.className = "stream-status " + (cls || "");
  }
}

function scheduleStreamRetry() {
  if (streamRetryTimer !== null) {
    return;
  }
  const delay = streamRetryMs;
  setStreamStatus("retry in " + Math.round(delay / 1000) + " s", "warn");
  streamRetryTimer = setTimeout(() => {
    streamRetryTimer = null;
    openStream();
  }, delay);
  streamRetryMs = Math.min(streamRetryMs * 2, STREAM_RETRY_MAX_MS);
}

function openStream() {
  if (stream) {
    stream.close();
    stream = null;
  }

  setStreamStatus("connecting", "warn");

  const source = new EventSource("/api/stream");
  stream = source;

  source.addEventListener("open", () => {
    if (stream !== source) {
      return;
    }
    streamRetryMs = STREAM_RETRY_MIN_MS;
    setStreamStatus("live", "ok");
    loadDevice();
  });

  source.addEventListener("state", (event) => {
    if (stream !== source) {
      return;
    }
    try {
      applyState(JSON.parse(event.data));
    } catch (error) {
      /* ignore a malformed frame */
    }
  });

  source.addEventListener("error", () => {
    if (stream !== source) {
      return;
    }
    /* The browser would retry on its own, but it keeps the old URL and has no
       backoff. Close it and retry ourselves so a device that moved from the AP
       address to its station address is picked up again. */
    source.close();
    stream = null;
    scheduleStreamRetry();
  });
}

function retryStreamNow() {
  if (stream) {
    return;
  }
  if (streamRetryTimer !== null) {
    clearTimeout(streamRetryTimer);
    streamRetryTimer = null;
  }
  streamRetryMs = STREAM_RETRY_MIN_MS;
  openStream();
}

window.addEventListener("online", retryStreamNow);
document.addEventListener("visibilitychange", () => {
  if (!document.hidden) {
    retryStreamNow();
  }
});

openStream();

async function loadDevice() {
  try {
    const res = await fetch("/api/device");
    const dev = await res.json();
    setText("device-name", dev.name || "");
    setText("d-name", dev.name || "?");
    setText("d-version", dev.version || "?");
    setText("d-git", dev.git || "?");
    setText("d-build", dev.build || "?");
    setText("d-board", dev.board || "?");
    setText("d-chip", dev.chip || "?");
  } catch (error) {
    /* placeholders stay */
  }
}

async function loadConfig() {
  try {
    const res = await fetch("/api/config");
    const cfg = await res.json();
    (cfg.networks || []).forEach((net, index) => {
      const input = $("ssid" + (index + 1));
      if (input && net.ssid) {
        input.value = net.ssid;
      }
    });
  } catch (error) {
    /* fields stay empty */
  }
}

async function loadSession() {
  try {
    const res = await fetch("/api/session");
    const session = await res.json();
    unlocked = Boolean(session.unlocked);
  } catch (error) {
    unlocked = false;
  }
  applyAccess();
}

$("lock-form").addEventListener("submit", async (event) => {
  event.preventDefault();

  const status = $("lock-status");
  const button = $("lock-button");
  const password = $("lock-pass").value;

  status.textContent = "";
  status.className = "";
  button.disabled = true;
  button.textContent = "Unlocking...";

  try {
    const res = await fetch("/api/login", { method: "POST", body: password });
    if (!res.ok) {
      throw new Error("wrong password");
    }
    unlocked = true;
    $("lock-pass").value = "";
    status.textContent = "Unlocked.";
    status.className = "ok";
  } catch (error) {
    status.textContent = "Wrong password.";
    status.className = "error";
  } finally {
    button.disabled = false;
    button.textContent = "Unlock";
    applyAccess();
  }
});

$("lock-now").addEventListener("click", async () => {
  try {
    await fetch("/api/logout", { method: "POST" });
  } catch (error) {
    /* the cookie is cleared on the next successful request */
  }

  unlocked = false;
  $("lock-status").textContent = "Locked.";
  $("lock-status").className = "";
  $("regenerate-password").classList.add("hidden");
  applyAccess();
});

$("regenerate").addEventListener("click", async () => {
  const status = $("lock-status");
  const output = $("regenerate-password");
  const button = $("regenerate");

  status.textContent = "";
  status.className = "";
  output.classList.add("hidden");
  button.disabled = true;
  button.textContent = "Regenerating...";

  try {
    const res = await fetch("/api/regenerate", { method: "POST" });
    if (res.status === 401) {
      unlocked = false;
      throw new Error("locked");
    }
    if (!res.ok) {
      throw new Error("regenerate failed");
    }
    const result = await res.json();
    output.textContent = "New AP password: " + result.password + " (shown once)";
    output.classList.remove("hidden");
    status.textContent = "Password changed. Other browsers are locked out.";
    status.className = "ok";
    unlocked = true;
  } catch (error) {
    status.textContent = "Regenerate failed.";
    status.className = "error";
  } finally {
    button.disabled = false;
    button.textContent = "Regenerate AP password";
    applyAccess();
  }
});

$("wifi-form").addEventListener("submit", async (event) => {
  event.preventDefault();

  const status = $("wifi-status");
  const button = $("wifi-save");
  const networks = [];

  for (let i = 1; i <= 2; i++) {
    const ssid = $("ssid" + i).value.trim();
    const pass = $("pass" + i).value;
    if (ssid) {
      networks.push({ ssid, pass });
    }
  }

  status.textContent = "";
  status.className = "";
  button.disabled = true;
  button.textContent = "Saving...";

  try {
    const res = await fetch("/api/wifi", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ networks }),
    });
    if (res.status === 401) {
      unlocked = false;
      applyAccess();
      throw new Error("locked");
    }
    if (!res.ok) {
      throw new Error("save failed");
    }
    status.textContent = "Saved. The portal stops and the device connects.";
    status.className = "ok";
  } catch (error) {
    status.textContent = "Saving failed.";
    status.className = "error";
    button.disabled = false;
    button.textContent = "Save and connect";
  }
});

$("ota-upload").addEventListener("click", () => {
  const file = $("ota-file").files[0];
  const status = $("ota-status");
  const progress = $("ota-progress");
  const button = $("ota-upload");

  if (!file) {
    status.textContent = "Choose a .bin file first.";
    status.className = "error";
    return;
  }

  status.textContent = "";
  status.className = "";
  progress.value = 0;
  button.disabled = true;

  const xhr = new XMLHttpRequest();
  xhr.open("POST", "/api/ota");

  xhr.upload.onprogress = (event) => {
    if (event.lengthComputable) {
      progress.value = (event.loaded / event.total) * 100;
    }
  };

  xhr.onload = () => {
    if (xhr.status === 200) {
      status.textContent = "Upload success, rebooting...";
      status.className = "ok";
    } else if (xhr.status === 401) {
      unlocked = false;
      applyAccess();
      status.textContent = "Locked. Unlock first.";
      status.className = "error";
      button.disabled = false;
    } else {
      status.textContent = "Upload failed: " + (xhr.responseText || "unknown error");
      status.className = "error";
      button.disabled = false;
    }
  };

  xhr.onerror = () => {
    status.textContent = "Upload error.";
    status.className = "error";
    button.disabled = false;
  };

  xhr.send(file);
});

// --- Touchscreen screenshot ---
let screenshotBlobUrl = null;

function rgb565ToCanvas(buffer, width, height) {
  const pixels = new Uint16Array(buffer);
  const canvas = $("screenshot-canvas");
  canvas.width = width;
  canvas.height = height;
  const ctx = canvas.getContext("2d");
  const imgData = ctx.createImageData(width, height);
  const data = imgData.data;

  for (let i = 0; i < width * height; i++) {
    // The panel returns RGB565 big-endian; Uint16Array reads little-endian.
    const raw = pixels[i];
    const rgb = ((raw & 0xff) << 8) | ((raw >> 8) & 0xff);
    const offset = i * 4;
    data[offset] = ((rgb >> 11) & 0x1f) << 3;
    data[offset + 1] = ((rgb >> 5) & 0x3f) << 2;
    data[offset + 2] = (rgb & 0x1f) << 3;
    data[offset + 3] = 255;
  }

  ctx.putImageData(imgData, 0, 0);
  return canvas;
}

$("screenshot-capture").addEventListener("click", async () => {
  const button = $("screenshot-capture");
  const save = $("screenshot-save");
  const preview = $("screenshot-preview");
  const placeholder = $("screenshot-placeholder");
  const status = $("screenshot-status");

  button.disabled = true;
  button.textContent = "Capturing...";
  status.textContent = "";
  status.className = "";

  try {
    const res = await fetch("/api/screenshot");
    if (!res.ok) {
      throw new Error("server returned " + res.status);
    }

    const width = Number(res.headers.get("X-Screenshot-Width")) || 320;
    const height = Number(res.headers.get("X-Screenshot-Height")) || 240;
    const buffer = await res.arrayBuffer();
    const expected = width * height * 2;

    if (buffer.byteLength < expected) {
      throw new Error("incomplete data: " + buffer.byteLength + " / " + expected);
    }

    const canvas = rgb565ToCanvas(buffer, width, height);
    preview.src = canvas.toDataURL("image/png");
    preview.style.display = "block";
    placeholder.style.display = "none";

    if (screenshotBlobUrl) {
      URL.revokeObjectURL(screenshotBlobUrl);
    }
    canvas.toBlob((blob) => {
      screenshotBlobUrl = URL.createObjectURL(blob);
    }, "image/png");

    save.disabled = false;
    status.textContent = "Captured " + width + "x" + height + ".";
    status.className = "ok";
  } catch (error) {
    status.textContent = "Capture failed: " + error.message;
    status.className = "error";
  } finally {
    button.disabled = false;
    button.textContent = "Capture";
  }
});

$("screenshot-save").addEventListener("click", () => {
  if (!screenshotBlobUrl) {
    return;
  }

  const a = document.createElement("a");
  a.href = screenshotBlobUrl;
  a.download = "iot12-display.png";
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
});

loadSession();
loadDevice();
loadConfig();
