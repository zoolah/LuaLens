#pragma once

inline const char* GRAPH_JS = R"GJS("use strict";
const W = 640, ROW = 18, HEAD = 48, GAP_X = 140, GAP_Y = 28;
const $ = id => document.getElementById(id);
const view = $("view"), world = $("world"), cardsEl = $("cards"), eg = $("eg"), input = $("input"), statusEl = $("status");
let model = null, cardEls = [], pathEls = [];
let cam = { x: 0, y: 0, s: 1 };
let lastBytes = [];
let mode = "graph";
const HELP = {
  graph: "scroll pans · ctrl-scroll zooms · drag a header to move it",
  linear: "all prototypes in order · click a name in the list to jump",
  raw: "full chunk bytes · 16-wide rows · ASCII on the right"
};

function say(msg, err) { statusEl.textContent = msg; statusEl.className = err ? "err" : ""; }

const pending = new Map();
let msgSeq = 0;
const SEP = String.fromCharCode(30);

function native(cmd, body) {
  return new Promise((resolve, reject) => {
    if (window.chrome && chrome.webview) {
      const id = String(++msgSeq);
      pending.set(id, { resolve, reject });
      chrome.webview.postMessage(id + SEP + cmd + SEP + (body || ""));
      return;
    }
    if (cmd === "compile") {
      fetch("/compile", { method: "POST", body: body || "" }).then(r => r.json()).then(resolve).catch(reject);
      return;
    }
    if (cmd === "sample") {
      fetch("/sample").then(r => r.text()).then(t => resolve({ ok: true, sample: t })).catch(reject);
      return;
    }
    reject(new Error("native host missing"));
  });
}
if (window.chrome && chrome.webview) {
  chrome.webview.addEventListener("message", e => {
    const msg = typeof e.data === "string" ? JSON.parse(e.data) : e.data;
    const p = pending.get(String(msg.id));
    if (!p) return;
    pending.delete(String(msg.id));
    p.resolve(msg);
  });
}

async function run() {
  say("compiling...");
  try {
    const j = await native("compile", input.value);
    if (!j.ok) { say(j.error, true); return; }
    lastBytes = j.bytes || [];
    render(analyze(j.root));
    renderLinear();
    renderHex();
    const ins = model.nodes.reduce((a, n) => a + n.ch.code.length, 0);
    say(model.nodes.length + " functions, " + ins + " ops, " + j.size + " bytes");
  } catch (e) { say(e.message, true); }
}

function layout(nodes) {
  const place = (n, y) => {
    n.x = n.depth * (W + GAP_X); n.y = y;
    let cy = y;
    for (const k of n.kids) cy = place(k, cy) + GAP_Y;
    return Math.max(y + n.h, cy - GAP_Y);
  };
  place(nodes[0], 0);
}

function cardHTML(n) {
  const c = n.ch;
  const rows = c.code.map((x, pc) => {
    const raw = rawWord(x);
    return `<div class="row c-${catOf(x[0])}${n.pseudo.has(pc) ? " pseudo" : ""}" data-pc="${pc}" title="line ${c.lines[pc] ?? "?"}  ${raw}">` +
      `<i>${pc}</i><span class="raw">${raw}</span><b>${OPS[x[0]] || "OP" + x[0]}</b><span>${esc(operands(n, pc))}</span><em>${esc(annotate(n, pc))}</em></div>`;
  }).join("");
  const ks = c.k.map((k, i) => {
    const v = fmtK(k);
    return `<div class="row k" title="${esc(v)}"><i>K${i}</i><b>${["nil", "bool", "?", "number", "string"][k[0]] || "?"}</b><span></span><em>${esc(v)}</em></div>`;
  }).join("");
  return `<div class="head"><div class="title">${esc(n.label)}<small>${n.kind === "main" ? esc(c.name) : n.kind || ""}</small></div>` +
    `<div class="meta">${protoMeta(n)}</div></div>` +
    `<div class="rows">${rows}</div>` + (c.k.length ? `<div class="khead">K (${c.k.length})</div><div class="ks">${ks}</div>` : "");
}

function protoMeta(n) {
  const c = n.ch;
  const params = c.args + " param" + (c.args === 1 ? "" : "s") + (c.varg & 2 ? " + ..." : "");
  const lines = c.first || c.last ? `<span>L${c.first}-${c.last}</span>` : "";
  const ups = c.nups ? `<span>${c.nups} up${c.nups === 1 ? "" : "s"}${c.upvals.length ? ": " + esc(c.upvals.join(", ")) : ""}</span>` : "";
  return `${lines}<span>${params}</span>${ups}<span>${c.stack} regs</span><span>${c.code.length} ops</span>`;
}

function renderLinear() {
  if (!model) { $("linear").innerHTML = ""; return; }
  $("linear").innerHTML = model.nodes.map(n => {
    const c = n.ch;
    const rows = c.code.map((x, pc) => {
      const raw = rawWord(x);
      return `<div class="row c-${catOf(x[0])}${n.pseudo.has(pc) ? " pseudo" : ""}">` +
        `<i>${pc}</i><span class="raw">${raw}</span><b>${OPS[x[0]] || "OP" + x[0]}</b><span>${esc(operands(n, pc))}</span><em>${esc(annotate(n, pc))}</em></div>`;
    }).join("");
    const ks = c.k.map((k, i) => {
      const v = fmtK(k);
      return `<div class="row k"><i>K${i}</i><b>${["nil", "bool", "?", "number", "string"][k[0]] || "?"}</b><span></span><em>${esc(v)}</em></div>`;
    }).join("");
    return `<section class="proto" id="p-${n.id}" data-depth="${Math.min(n.depth + 1, 3)}" style="margin-left:${n.depth * 16}px">` +
      `<div class="phead"><div class="title">${esc(n.label)}<small>${n.kind === "main" ? esc(c.name) : n.kind || ""}</small></div>` +
      `<div class="meta">${protoMeta(n)}</div></div>` +
      `<div class="rows">${rows}</div>` +
      (c.k.length ? `<div class="khead">K (${c.k.length})</div><div class="ks">${ks}</div>` : "") +
      `</section>`;
  }).join("");
}

function renderHex() {
  const b = lastBytes, cols = 16;
  if (!b.length) { $("hex").innerHTML = ""; return; }
  const head = `<div class="hhead"><span class="hoff">off</span>` +
    Array.from({ length: cols }, (_, i) => `<span class="hbytes">${i.toString(16).toUpperCase().padStart(2, "0")}</span>`).join("") +
    `<span></span><span class="hascii">ascii</span></div>`;
  let rows = "";
  for (let i = 0; i < b.length; i += cols) {
    const slice = b.slice(i, i + cols);
    const hex = slice.map(n => `<span class="hbytes">0x${n.toString(16).toUpperCase().padStart(2, "0")}</span>`).join("") +
      Array.from({ length: cols - slice.length }, () => `<span class="hbytes"></span>`).join("");
    const asc = slice.map(n => n >= 32 && n < 127 ? String.fromCharCode(n) : ".").join("");
    rows += `<div class="hrow"><span class="hoff">${i.toString(16).toUpperCase().padStart(4, "0")}</span>${hex}<span></span><span class="hascii">${esc(asc.padEnd(cols, " "))}</span></div>`;
  }
  $("hex").innerHTML = head + rows;
}

function setMode(m) {
  mode = m;
  document.body.dataset.mode = m;
  [...$("tabs").children].forEach(b => b.classList.toggle("on", b.dataset.mode === m));
  $("help").textContent = HELP[m] || HELP.graph;
  if (m === "graph" && model) requestAnimationFrame(fit);
}

function render(m) {
  model = m;
  cardsEl.innerHTML = m.nodes.map(n =>
    `<div class="card" data-id="${n.id}" data-depth="${Math.min(n.depth, 3)}" style="width:${W}px">${cardHTML(n)}</div>`).join("");
  cardEls = [...cardsEl.children];
  m.nodes.forEach(n => { n.h = cardEls[n.id].offsetHeight; });
  layout(m.nodes);
  m.nodes.forEach(n => moveCard(n));
  m.edges.forEach(e => {
    const row = cardEls[e.from.id].querySelector(`.row[data-pc="${e.pc}"]`);
    if (row) row.classList.add("lk-" + e.kind);
  });
  drawEdges();
  $("list").innerHTML = m.nodes.map(n =>
    `<li data-id="${n.id}" style="padding-left:${n.depth * 12}px"><span>${esc(n.label)}</span><span>${n.ch.code.length}</span></li>`).join("");
  $("empty").style.display = "none";
  fit();
}

function moveCard(n) { const el = cardEls[n.id]; el.style.left = n.x + "px"; el.style.top = n.y + "px"; }

function drawEdges() {
  eg.innerHTML = model.edges.map((e, i) => {
    const a = e.from, b = e.to, sy = a.y + HEAD + e.pc * ROW + ROW / 2, ty = b.y + 24;
    let sx, tx, ds, dt;
    if (b.x > a.x) { sx = a.x + W; tx = b.x; ds = 1; dt = -1; }
    else if (b.x < a.x) { sx = a.x; tx = b.x + W; ds = -1; dt = 1; }
    else { sx = a.x + W; tx = b.x + W; ds = 1; dt = 1; }
    const off = Math.max(60, Math.abs(tx - sx) / 2);
    return `<path class="edge ${e.kind}" data-i="${i}" marker-end="url(#arr-${e.kind})" d="M${sx},${sy} C${sx + ds * off},${sy} ${tx + dt * off},${ty} ${tx},${ty}"/>` +
           `<circle class="dot ${e.kind}" cx="${sx}" cy="${sy}" r="3"/>`;
  }).join("");
  pathEls = [...eg.querySelectorAll(".edge")];
}

function lit(on, e) {
  const t = e.target.closest && e.target.closest(".row[data-pc]");
  if (!t || !model) return;
  const id = +t.closest(".card").dataset.id, pc = +t.dataset.pc;
  model.edges.forEach((ed, i) => {
    if (ed.from.id === id && ed.pc === pc) {
      pathEls[i].classList.toggle("hot", on);
      cardEls[ed.to.id].classList.toggle("hot", on);
    }
  });
}
cardsEl.addEventListener("mouseover", e => lit(true, e));
cardsEl.addEventListener("mouseout", e => lit(false, e));

function apply() {
  world.style.transform = `translate(${cam.x}px,${cam.y}px) scale(${cam.s})`;
  view.style.backgroundPosition = `${cam.x}px ${cam.y}px`;
  view.style.backgroundSize = `${32 * cam.s}px ${32 * cam.s}px`;
  $("zoom").textContent = Math.round(cam.s * 100) + "%";
}
function zoomAt(mx, my, f) {
  const s = Math.min(2.5, Math.max(0.05, cam.s * f)), k = s / cam.s;
  cam.x = mx - (mx - cam.x) * k; cam.y = my - (my - cam.y) * k; cam.s = s; apply();
}
function fit() {
  if (!model) return;
  let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
  for (const n of model.nodes) {
    minX = Math.min(minX, n.x); minY = Math.min(minY, n.y);
    maxX = Math.max(maxX, n.x + W); maxY = Math.max(maxY, n.y + n.h);
  }
  const vw = view.clientWidth, vh = view.clientHeight;
  cam.s = Math.min(1.2, Math.max(0.08, Math.min((vw - 80) / Math.max(1, maxX - minX), (vh - 80) / Math.max(1, maxY - minY))));
  cam.x = (vw - (maxX + minX) * cam.s) / 2;
  cam.y = (vh - (maxY + minY) * cam.s) / 2;
  apply();
}
function focusNode(n) {
  cam.s = 1;
  cam.x = view.clientWidth / 2 - (n.x + W / 2) * cam.s;
  cam.y = 50 - n.y * cam.s; apply();
  cardEls[n.id].classList.add("hot");
  setTimeout(() => cardEls[n.id].classList.remove("hot"), 900);
}

view.addEventListener("wheel", e => {
  e.preventDefault();
  if (e.ctrlKey || e.metaKey) {
    const r = view.getBoundingClientRect();
    zoomAt(e.clientX - r.left, e.clientY - r.top, Math.exp(-e.deltaY * 0.0025));
  } else {
    cam.x -= e.shiftKey ? e.deltaY : e.deltaX;
    cam.y -= e.shiftKey ? 0 : e.deltaY;
    apply();
  }
}, { passive: false });

let drag = null;
view.addEventListener("pointerdown", e => {
  if (e.button !== 0 || e.target.closest("#tools")) return;
  const head = e.target.closest(".head");
  if (head) {
    const n = model.nodes[+head.parentElement.dataset.id];
    drag = { n, sx: e.clientX, sy: e.clientY, ox: n.x, oy: n.y };
  } else if (e.target.closest(".rows, .ks, .khead")) return;
  else {
    drag = { sx: e.clientX, sy: e.clientY, ox: cam.x, oy: cam.y };
    view.classList.add("dragging");
  }
  view.setPointerCapture(e.pointerId);
  e.preventDefault();
});
view.addEventListener("pointermove", e => {
  if (!drag) return;
  const dx = e.clientX - drag.sx, dy = e.clientY - drag.sy;
  if (drag.n) { drag.n.x = drag.ox + dx / cam.s; drag.n.y = drag.oy + dy / cam.s; moveCard(drag.n); drawEdges(); }
  else { cam.x = drag.ox + dx; cam.y = drag.oy + dy; apply(); }
});
const endDrag = () => { drag = null; view.classList.remove("dragging"); };
view.addEventListener("pointerup", endDrag);
view.addEventListener("pointercancel", endDrag);

$("go").onclick = run;
$("fit").onclick = fit;
$("zin").onclick = () => zoomAt(view.clientWidth / 2, view.clientHeight / 2, 1.25);
$("zout").onclick = () => zoomAt(view.clientWidth / 2, view.clientHeight / 2, 0.8);
$("tabs").onclick = e => {
  const b = e.target.closest("button[data-mode]");
  if (b) setMode(b.dataset.mode);
};
$("open").onclick = async () => {
  if (window.chrome && chrome.webview) {
    try {
      const j = await native("open");
      if (!j || j.cancel) return;
      if (!j.ok) { say(j.error || "open failed", true); return; }
      input.value = j.source || "";
      $("fname").textContent = j.name || "untitled.lua";
      say("loaded " + (j.name || "file"));
    } catch (e) { say(e.message, true); }
    return;
  }
  $("file").click();
};
$("file").addEventListener("change", async e => {
  const f = e.target.files && e.target.files[0];
  if (!f) return;
  input.value = await f.text();
  $("fname").textContent = f.name;
  say("loaded " + f.name);
  e.target.value = "";
});
$("sample").onclick = async () => {
  try {
    const j = await native("sample");
    input.value = j.sample || "";
    $("fname").textContent = "sample.lua";
    say("sample loaded");
  } catch (e) { say(e.message, true); }
};
$("list").onclick = e => {
  const li = e.target.closest("li");
  if (!li || !model) return;
  const n = model.nodes[+li.dataset.id];
  if (mode === "graph") focusNode(n);
  else if (mode === "linear") {
    const el = $("p-" + n.id);
    if (el) el.scrollIntoView({ behavior: "smooth", block: "start" });
  }
};
input.addEventListener("keydown", e => {
  if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) { e.preventDefault(); run(); }
});
window.addEventListener("resize", () => { if (model) apply(); });
)GJS";
