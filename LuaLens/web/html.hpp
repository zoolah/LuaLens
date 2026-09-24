#pragma once

inline const char* INDEX_HTML = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>LuaLens</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body data-mode="graph">
<header id="top">
  <div class="brand"><h1>LuaLens</h1></div>
  <nav id="tabs">
    <button type="button" data-mode="graph" class="on">graph</button>
    <button type="button" data-mode="linear">linear</button>
    <button type="button" data-mode="raw">raw</button>
  </nav>
</header>
<div id="work">
<aside id="side">
  <div class="editor">
    <div class="ebar">
      <span id="fname">untitled.lua</span>
      <span class="grow"></span>
      <button type="button" id="open">open</button>
      <button type="button" id="sample">sample</button>
      <input id="file" type="file" accept=".lua,.txt,text/plain" hidden>
    </div>
    <textarea id="input" spellcheck="false" placeholder="-- Lua 5.1&#10;print('hello')"></textarea>
  </div>
  <p class="btns"><button id="go">compile &amp; disassemble</button></p>
  <div id="status"></div>
  <ul id="list"></ul>
</aside>
<div id="stage">
<main id="view">
  <div id="world">
    <svg id="edges" width="1" height="1">
      <defs>
        <marker id="arr-child" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto"><path d="M0,1 L9,5 L0,9 z" fill="#9b7ed8"/></marker>
        <marker id="arr-call" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto"><path d="M0,1 L9,5 L0,9 z" fill="#c46a9a"/></marker>
      </defs>
      <g id="eg"></g>
    </svg>
    <div id="cards"></div>
  </div>
  <div id="empty">no chunk loaded</div>
  <div id="tools">
    <button id="zout">-</button>
    <span id="zoom">100%</span>
    <button id="zin">+</button>
    <button id="fit">fit</button>
  </div>
</main>
<div id="linear" class="pane"></div>
<div id="hex" class="pane"></div>
</div>
</div>
</body>
</html>
)HTML";
