const elements = {
  temperature: document.getElementById("temperature"),
  brightness: document.getElementById("brightness"),
  fanStatus: document.getElementById("fanStatus"),
  fanHint: document.getElementById("fanHint"),
  shadeStatus: document.getElementById("shadeStatus"),
  shadeHint: document.getElementById("shadeHint"),
  updatedAt: document.getElementById("updatedAt"),
  cameraFrame: document.getElementById("cameraFrame"),
  cameraLink: document.getElementById("cameraLink"),
  cameraEmpty: document.getElementById("cameraEmpty")
};

function formatTime(isoString) {
  if (!isoString) {
    return "--";
  }

  const date = new Date(isoString);
  return date.toLocaleString("zh-CN", {
    year: "numeric",
    month: "2-digit",
    day: "2-digit",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit"
  });
}

function render(state) {
  elements.temperature.textContent =
    typeof state.temperature === "number" ? state.temperature.toFixed(1) : "--";
  elements.brightness.textContent =
    typeof state.brightness === "number" ? Math.round(state.brightness) : "--";

  elements.fanStatus.textContent = state.fanOn ? "Running" : "Off";
  elements.fanHint.textContent = state.fanOn
    ? "Temperature is above threshold, fan is cooling the greenhouse."
    : "Temperature is below threshold, fan is on standby.";

  elements.shadeStatus.textContent = state.shadeClosed ? "Closed" : "Open";
  elements.shadeHint.textContent = state.shadeClosed
    ? "Brightness is high, the shade panel is blocking sunlight."
    : "Brightness is low, the shade panel is allowing sunlight in.";

  elements.updatedAt.textContent = formatTime(state.updatedAt);

  if (state.cameraUrl) {
    if (elements.cameraFrame.src !== state.cameraUrl) {
      elements.cameraFrame.src = state.cameraUrl;
    }

    elements.cameraFrame.style.display = "block";
    elements.cameraLink.href = state.cameraUrl;
    elements.cameraEmpty.style.display = "none";
  } else {
    elements.cameraFrame.removeAttribute("src");
    elements.cameraFrame.style.display = "none";
    elements.cameraLink.href = "#";
    elements.cameraEmpty.style.display = "grid";
  }
}

async function bootstrap() {
  const response = await fetch("/api/state");
  const initialState = await response.json();
  render(initialState);
}

bootstrap().catch((error) => {
  console.error("Failed to load dashboard state:", error);
});

const stream = new EventSource("/api/stream");
stream.onmessage = (event) => {
  render(JSON.parse(event.data));
};

stream.onerror = () => {
  console.warn("State stream disconnected, dashboard will keep retrying.");
};
