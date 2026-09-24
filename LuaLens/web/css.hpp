#pragma once

inline const char* STYLE_CSS = R"CSS(
*, *::before, *::after { box-sizing: border-box; }
html, body { height: 100%; margin: 0; color-scheme: dark; }
body {
  display: flex;
  flex-direction: column;
  overflow: hidden;
  color: #e8e2f0;
  font: 12px/1.4 Verdana, Geneva, sans-serif;
  background: #110f16;
}
#top {
  flex: none;
  height: 42px;
  display: flex;
  align-items: stretch;
  padding: 0 12px 0 14px;
  background: #16141c;
  border-bottom: 1px solid #3a3648;
  z-index: 3;
}
.brand { display: flex; align-items: center; min-width: 140px; }
h1 {
  margin: 0;
  font: italic 18px/1 Georgia, "Times New Roman", serif;
  color: #f0e8fa;
}
#tabs { display: flex; margin-left: auto; }
#tabs button {
  border: none;
  border-bottom: 2px solid transparent;
  background: transparent;
  border-radius: 0;
  padding: 0 16px;
  color: #9a92a8;
}
#tabs button:hover { background: #1b1922; color: #e8e2f0; }
#tabs button.on { color: #f0e8fa; border-bottom-color: #9b7ed8; }
#work { flex: 1; display: flex; min-height: 0; min-width: 0; }
#side {
  width: 380px;
  flex: none;
  display: flex;
  flex-direction: column;
  gap: 8px;
  padding: 12px;
  background: #1b1922;
  border-right: 1px solid #3a3648;
  overflow-y: auto;
  z-index: 2;
}
.editor {
  flex: 1 1 auto;
  min-height: 240px;
  display: flex;
  flex-direction: column;
  background: #110f16;
  border: 1px solid #3a3648;
}
.ebar {
  display: flex;
  align-items: center;
  gap: 8px;
  flex: none;
  padding: 5px 8px;
  background: #1e1c26;
  border-bottom: 1px solid #3a3648;
  font-size: 11px;
  color: #9a92a8;
}
.elabel {
  font-size: 10px;
  letter-spacing: .08em;
  text-transform: uppercase;
  color: #9b7ed8;
}
#fname { color: #c8c0d4; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; max-width: 160px; }
.grow { flex: 1; }
.ebar button { padding: 2px 8px; font-size: 11px; }
textarea {
  width: 100%;
  flex: 1 1 auto;
  min-height: 180px;
  height: auto;
  resize: none;
  padding: 10px 12px;
  font: 12px/1.5 Consolas, "Courier New", monospace;
  color: #e8e2f0;
  background: transparent;
  border: none;
  outline: none;
}
textarea::placeholder { color: #6e6a80; }
.btns { display: flex; gap: 6px; margin: 0; }
button {
  font: 12px Verdana, Geneva, sans-serif;
  padding: 3px 10px;
  color: #e8e2f0;
  background: #2e2a38;
  border: 1px solid #4a4658;
  cursor: pointer;
}
button:hover { background: #3a3648; }
button:active { background: #f0e8fa; color: #110f16; }
#go { background: #4a3870; color: #f0e8fa; border-color: #6a52a0; }
#go:hover { background: #5c4688; color: #f0e8fa; }
#status { min-height: 16px; color: #9a92a8; }
#status.err { color: #e07090; }
#list { margin: 4px 0 0; padding: 0; list-style: none; }
#list li {
  display: flex;
  justify-content: space-between;
  gap: 8px;
  padding: 2px 0;
  cursor: pointer;
  white-space: nowrap;
}
#list li:hover { text-decoration: underline; }
#list li span:first-child { overflow: hidden; text-overflow: ellipsis; }
#list li span:last-child { color: #9a92a8; }
#help { margin: auto 0 0; padding-top: 10px; font-size: 11px; color: #9a92a8; }

#stage { position: relative; flex: 1; min-width: 0; min-height: 0; }
#view {
  position: absolute;
  inset: 0;
  overflow: hidden;
  cursor: grab;
  touch-action: none;
  background-color: #110f16;
  background-image:
    linear-gradient(#24202c 1px, transparent 1px),
    linear-gradient(90deg, #24202c 1px, transparent 1px);
  background-size: 32px 32px;
}
.pane {
  display: none;
  position: absolute;
  inset: 0;
  overflow: auto;
  background: #110f16;
  padding: 18px 22px 40px;
}
body[data-mode="linear"] #linear,
body[data-mode="raw"] #hex { display: block; }
body[data-mode="linear"] #view,
body[data-mode="raw"] #view,
body[data-mode="linear"] #tools,
body[data-mode="raw"] #tools,
body[data-mode="linear"] #empty,
body[data-mode="raw"] #empty { display: none; }
#view.dragging { cursor: grabbing; }
#world { position: absolute; left: 0; top: 0; transform-origin: 0 0; }
#edges { position: absolute; left: 0; top: 0; overflow: visible; pointer-events: none; }
#cards { position: absolute; left: 0; top: 0; }
.edge { fill: none; stroke-width: 1.5; opacity: .75; }
.edge.child { stroke: #9b7ed8; }
.edge.call { stroke: #c46a9a; stroke-dasharray: 5 4; }
.edge.hot { opacity: 1; stroke-width: 2.4; }
.dot.child { fill: #9b7ed8; }
.dot.call { fill: #c46a9a; }

.card {
  position: absolute;
  background: #1a1822;
  border: 1px solid #322e42;
  box-shadow: 0 10px 28px rgba(8, 6, 16, .45);
  overflow: hidden;
  cursor: default;
  color: #e8e2f0;
  padding-bottom: 8px;
}

.card.hot { outline: 2px solid #c46a9a; outline-offset: -1px; }
.head {
  height: 48px;
  padding: 10px 14px 0;
  background: #221f2c;
  border-bottom: 1px solid #2e2a3a;
  cursor: move;
}
.title { display: flex; align-items: baseline; gap: 8px; font-weight: 600; font-size: 13px; letter-spacing: .01em; white-space: nowrap; }
.title small { font-weight: normal; font-size: 11px; color: #9a92a8; }
.meta { margin-top: 3px; font-size: 11px; color: #9a92a8; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.meta span + span { margin-left: 10px; }
.row {
  display: grid;
  grid-template-columns: 32px 148px 92px 88px minmax(0, 1fr);
  align-items: center;
  height: 18px;
  padding: 0 12px 0 8px;
  font: 11px Consolas, "Courier New", monospace;
  white-space: nowrap;
}
.row > * { min-width: 0; overflow: hidden; text-overflow: ellipsis; }
.row .raw { color: #6e6a80; overflow: visible; text-overflow: clip; letter-spacing: 0; }
.row:hover { background: #2a2634; }
.row i { font-style: normal; color: #6e6a80; text-align: right; padding-right: 8px; }
.row b { font-weight: 600; }
.row em { font-style: normal; color: #9a92a8; }
.row.pseudo { opacity: .55; }
.c-ar b { color: #c4a5e8; }
.c-ct b { color: #9b7ed8; }
.c-cl b { color: #d46ab8; }
.c-fn b { color: #8aa0e0; }
.row.lk-child { box-shadow: inset 3px 0 #9b7ed8; }
.row.lk-call { box-shadow: inset 3px 0 #c46a9a; }
.khead {
  height: 22px;
  padding: 4px 14px 0;
  font-size: 10px;
  font-weight: 600;
  letter-spacing: .06em;
  text-transform: uppercase;
  color: #9a92a8;
  border-top: 1px solid #2e2a3a;
  background: transparent;
}
.row.k { grid-template-columns: 36px 72px 8px minmax(0, 1fr); }
.row.k b { color: #9a92a8; font-weight: normal; }
.row.k em { color: #c8c0d4; }

#tools {
  position: absolute;
  right: 12px;
  bottom: 12px;
  display: flex;
  align-items: center;
  gap: 4px;
  padding: 4px;
  background: #1b1922;
  border: 1px solid #4a4658;
  cursor: default;
}
#tools button { padding: 2px 8px; }
#zoom { min-width: 40px; text-align: center; font-size: 11px; color: #9a92a8; }
#empty {
  position: absolute;
  left: 50%;
  top: 42%;
  transform: translate(-50%, -50%);
  color: #6e6a80;
  font: italic 16px Georgia, "Times New Roman", serif;
  pointer-events: none;
}
.proto {
  margin: 0 0 22px;
  background: #1a1822;
  border: 1px solid #322e42;
  overflow: hidden;
}

.proto .phead {
  padding: 10px 14px 8px;
  background: #221f2c;
  border-bottom: 1px solid #2e2a3a;
}
.proto .phead .title { font-size: 13px; }
#linear .row { height: auto; min-height: 18px; }
#hex {
  font: 12px/1.7 Consolas, "Courier New", monospace;
}
.hhead, .hrow {
  display: grid;
  grid-template-columns: 64px repeat(16, 2.7em) 16px 132px;
  align-items: center;
  gap: 0;
  padding: 0 8px;
  min-width: 720px;
}
.hhead { color: #6e6a80; font-size: 10px; letter-spacing: .04em; margin-bottom: 6px; }
.hrow:nth-child(odd) { background: #18151e; }
.hrow:hover { background: #2a2634; }
.hoff { color: #9b7ed8; }
.hbytes { font-style: normal; color: #c8c0d4; text-align: center; }
.hascii { color: #6e6a80; white-space: pre; }
@media (max-width: 800px) { #side { width: 280px; } }
)CSS";
