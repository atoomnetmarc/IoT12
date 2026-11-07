const $ = (id) => document.getElementById(id);

let unlocked = false;
let portalActive = false;
let lastUptime = null;
let lastMinionUptime = null;
let deviceLoading = false;

function setText(id, text) {
  const el = $(id);
  if (el) {
    el.textContent = text;
  }
}

function fmt(value, digits) {
  return Number.isFinite(value) ? value.toFixed(digits) : "--";
}

function fmtUptime(seconds) {
  if (!Number.isFinite(seconds)) {
    return "--";
  }
  const total = Math.floor(seconds);
  const days = Math.floor(total / 86400);
  const hours = Math.floor((total % 86400) / 3600);
  const minutes = Math.floor((total % 3600) / 60);
  const secs = total % 60;
  return days + "d " + String(hours).padStart(2, "0") + ":" + String(minutes).padStart(2, "0") + ":" + String(secs).padStart(2, "0");
}

function fmtBytes(bytes) {
  if (!Number.isFinite(bytes)) {
    return "--";
  }
  if (bytes >= 1024 * 1024) {
    return (bytes / (1024 * 1024)).toFixed(2) + " MiB";
  }
  if (bytes >= 1024) {
    return (bytes / 1024).toFixed(1) + " KiB";
  }
  return bytes + " B";
}

function fmtHex(value, digits) {
  if (!Number.isFinite(value)) {
    return "--";
  }
  return "0x" + Math.trunc(value).toString(16).padStart(digits, "0");
}

const INT_NAMES = [
  [31, "REBOOT"],
  [30, "GROUP_A"],
  [29, "GROUP_B"],
  [28, "LINK_TIMEOUT"],
];

function fmtInterrupt(value) {
  if (!Number.isFinite(value)) {
    return "--";
  }
  const names = INT_NAMES.filter(([bit]) => (value & (1 << bit)) !== 0).map(([, name]) => name);
  return fmtHex(value, 8) + (names.length ? " " + names.join("|") : "");
}

const SHAKE_PHASE = ["ACTIVE", "RAMP", "STANDBY2", "EXPIRED"];
const FLASH_MODES = ["QIO", "QOUT", "DIO", "DOUT", "FAST_READ", "SLOW_READ"];

function flashModeName(mode) {
  return FLASH_MODES[mode] || "?";
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

  setText("d-uptime", fmtUptime(state.uptime));
  setText("d-uptime-nerd", fmtUptime(state.uptime));
  setText("d-total", fmtUptime(state.total));
  setText("d-boots", state.boots ?? "--");
  setText("d-reset", state.reset || "--");
  setText("d-heap", fmtBytes(state.heap));
  setText("d-heap-free", fmtBytes(state.heap));
  setText("d-heap-min", fmtBytes(state.heap_min));
  setText("d-heap-max", fmtBytes(state.heap_max));
  setText("d-heap-size", fmtBytes(state.heap_size));
  setText("d-die", fmt(state.die, 1) + " C");

  setText("m-uptime", fmtUptime(state.minionUptime));
  setText("m-reset", state.minionReset || "--");
  setText("m-free-ram", fmtBytes(state.minionFreeRam));
  setText("m-events", state.minionEvents ?? "--");
  setText("m-die", fmt(state.minionDie, 1) + " C");
  setText("m-vdd", fmt(state.minionVdd, 2) + " V");

  setText("l-age", (state.linkAge ?? "--") + " ms");
  setText("l-heartbeats", state.heartbeats ?? "--");
  setText("l-errors", state.errors ?? "--");
  setText("l-rewrites", state.rewrites ?? "--");
  setText("l-forced-off", state.forcedOff ?? "--");
  setText("l-last-int", fmtInterrupt(state.lastInt));
  setText("l-compatible", state.compatible ? "yes" : "no");

  setText("h-cycles", state.cycles ?? "--");
  setText("h-adc-ms", (state.adcMs ?? "--") + " ms");
  setText("h-pwm-duty", state.pwmDuty ?? "--");
  setText("h-pwm-period", state.pwmPeriod ?? "--");
  setText("h-p", fmt(state.p, 4));
  setText("h-i", fmt(state.i, 4));
  setText("h-d", fmt(state.d, 4));
  setText("h-heatfactor", fmt(state.heatfactor, 4));
  setText("h-kp", fmt(state.kp, 5));
  setText("h-ki", fmt(state.ki, 5));
  setText("h-kd", fmt(state.kd, 6));

  setText("s-voltage", fmt(state.shakeV, 3) + " V");
  setText("s-phase", SHAKE_PHASE[state.shakePhase] || "--");
  setText("s-raw", state.shakeRaw ? "high" : "low");
  setText("s-debounced", state.shakeDebounced ? "high" : "low");
  setText("s-expired", state.shakeExpired ? "yes" : "no");
  setText("s-age", fmtUptime((state.shakeAge ?? 0) / 1000));

  setText("c-target", fmt(state.set, 1) + " C");

  if (lastUptime !== null && state.uptime < lastUptime) {
    loadDevice();
  }
  if (lastMinionUptime !== null && state.minionUptime > 0 && state.minionUptime < lastMinionUptime) {
    loadDevice();
  }
  lastUptime = state.uptime;
  if (state.minionUptime > 0) {
    lastMinionUptime = state.minionUptime;
  }

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
  if (deviceLoading) {
    return;
  }
  deviceLoading = true;
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
    setText("d-chip-rev", dev.chipRevision ?? "?");
    setText("d-cores", dev.cores ?? "?");
    setText("d-cpu", (dev.cpuMhz ?? "?") + " MHz");
    setText("d-sdk", dev.sdk || "?");
    setText("d-flash", fmtBytes(dev.flashSize));
    setText("d-flash-speed", dev.flashSpeed ? (dev.flashSpeed / 1000000).toFixed(0) + " MHz" : "?");
    setText("d-flash-mode", flashModeName(dev.flashMode));
    setText("d-sketch", fmtBytes(dev.sketchSize));
    setText("d-free-sketch", fmtBytes(dev.freeSketch));
    setText("d-mac", dev.mac || "?");
    setText("d-ap-mac", dev.apMac || "?");
    setText("d-hostname", dev.hostname || "?");

    const minion = dev.minion || {};
    setText("m-version", minion.version || "?");
    setText("m-board", minion.board || "?");
    setText("m-build", minion.build || "?");
    setText("m-sketch", fmtBytes(minion.sketchSize));
    setText("m-free-sketch", fmtBytes(minion.freeSketch));
    setText("m-serial", minion.serial || "?");
    setText("m-const1", minion.const1 !== undefined ? fmtHex(minion.const1, 4) : "?");
    setText("m-const2", fmt(minion.const2, 6));
    setText("l-timeout", (minion.linkTimeout ?? "?") + " ms");
    setText("c-ntc-pullup", fmt(minion.ntcPullup, 0) + " ohm");
    setText("c-heater-shunt", fmt(minion.heaterShunt, 4) + " ohm");
    setText("c-gain-divider", fmt(minion.gainDivider, 5));
    setText("c-gain-temp", fmt(minion.gainTemp, 2));
    setText("c-gain-current", fmt(minion.gainCurrent, 2));
    setText("c-tc-offset", fmt(minion.thermocoupleOffset, 6) + " V");
    setText("c-standby1", fmt(minion.standby1 - 273.15, 1) + " C");
    setText("c-standby2", fmt(minion.standby2 - 273.15, 1) + " C");
    setText("c-shake1", fmt(minion.shakeStage1 / 1000, 0) + " s");
    setText("c-shake2", fmt(minion.shakeStage2 / 1000, 0) + " s");
    setText("c-shake-off", fmt(minion.shakeOff / 1000, 0) + " s");
  } catch (error) {
    /* placeholders stay */
  } finally {
    deviceLoading = false;
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
