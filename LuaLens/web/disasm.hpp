#pragma once

inline const char* DISASM_JS = R"DJS("use strict";
const OPS = ["MOVE","LOADK","LOADBOOL","LOADNIL","GETUPVAL","GETGLOBAL","GETTABLE","SETGLOBAL","SETUPVAL","SETTABLE",
  "NEWTABLE","SELF","ADD","SUB","MUL","DIV","MOD","POW","UNM","NOT","LEN","CONCAT","JMP","EQ","LT","LE","TEST","TESTSET",
  "CALL","TAILCALL","RETURN","FORLOOP","FORPREP","TFORLOOP","SETLIST","CLOSE","CLOSURE","VARARG"];
const USES = ["AB","ABx","ABC","AB","AB","ABx","ABC","ABx","AB","ABC","ABC","ABC","ABC","ABC","ABC","ABC","ABC","ABC",
  "AB","AB","AB","ABC","sBx","ABC","ABC","ABC","AC","ABC","ABC","ABC","AB","AsBx","AsBx","AC","ABC","A","ABx","AB"];
const RK_B = new Set([9,12,13,14,15,16,17,23,24,25]);
const RK_C = new Set([6,9,11,12,13,14,15,16,17,23,24,25]);
const catOf = op => op >= 12 && op <= 21 ? "ar" : (op >= 22 && op <= 27) || (op >= 31 && op <= 33) ? "ct"
  : op === 28 || op === 29 || op === 30 || op === 37 ? "cl" : op >= 34 && op <= 36 ? "fn" : "mv";

const esc = s => String(s).replace(/[&<>"]/g, c => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;" }[c]));
const fmtK = k => !k ? "?" : k[0] === 4 ? JSON.stringify(k[1]) : k[0] === 0 ? "nil" : String(k[1]);
const rawWord = x => {
  const w = (x[0] | (x[1] << 6) | (x[4] << 14)) >>> 0;
  return [w & 255, (w >>> 8) & 255, (w >>> 16) & 255, (w >>> 24) & 255]
    .map(n => "0x" + n.toString(16).toUpperCase().padStart(2, "0")).join(" ");
};

function analyze(root) {
  const nodes = [], globals = new Map(), edges = [], seen = new Set();

  (function walk(ch, parent, depth) {
    const n = { id: nodes.length, ch, parent, depth, kids: [], name: null, kind: null, via: null, pseudo: new Set() };
    nodes.push(n);
    if (parent) parent.kids.push(n);
    n.locs = ch.locals.map((l, i, a) => ({
      name: l[0], s: l[1], e: l[2], fn: null,
      reg: a.slice(0, i).filter(p => p[2] > l[1]).length,
    }));
    ch.protos.forEach(p => walk(p, n, depth + 1));
  })(root, null, 0);

  for (const n of nodes) {
    const code = n.ch.code, K = n.ch.k;
    code.forEach((x, pc) => {
      if (x[0] !== 36) return;
      const child = n.kids[x[4]];
      if (!child) return;
      const nu = child.ch.nups;
      for (let i = 1; i <= nu; i++) n.pseudo.add(pc + i);
      child.via = pc;
      const next = code[pc + 1 + nu];
      const loc = n.locs.find(l => l.reg === x[1] && l.s >= pc && l.s <= pc + 1 + nu);
      if (loc) { child.name = loc.name; child.kind = "local"; loc.fn = child; }
      else if (next && next[0] === 7 && next[1] === x[1] && K[next[4]]) {
        child.name = String(K[next[4]][1]); child.kind = "global"; globals.set(child.name, child);
      } else if (next && next[0] === 9 && next[3] === x[1] && next[2] >= 256 && K[next[2] - 256] && K[next[2] - 256][0] === 4) {
        child.name = K[next[2] - 256][1]; child.kind = "field";
      }
    });
  }
  for (const n of nodes) {
    n.label = n.name || (n.parent ? "anonymous" : "main chunk");
    if (!n.parent) n.kind = "main";
  }

  const add = (from, pc, to, kind) => {
    const key = from.id + ":" + pc + ":" + to.id;
    if (!seen.has(key)) { seen.add(key); edges.push({ from, pc, to, kind }); }
  };
  for (const n of nodes) {
    n.kids.forEach(k => { if (k.via !== null) add(n, k.via, k, "child"); });
    n.ch.code.forEach((x, pc) => {
      if (n.pseudo.has(pc)) return;
      if (x[0] === 5) {
        const k = n.ch.k[x[4]], t = k && k[0] === 4 && globals.get(k[1]);
        if (t) add(n, pc, t, "call");
      } else if (x[0] === 4) {
        const u = n.ch.upvals[x[2]];
        for (let p = u && n.parent; p; p = p.parent) {
          const l = p.locs.find(l => l.name === u && l.fn);
          if (l) { add(n, pc, l.fn, "call"); break; }
        }
      } else if (x[0] === 0) {
        const l = n.locs.find(l => l.reg === x[2] && l.s <= pc && pc < l.e && l.fn);
        if (l) add(n, pc, l.fn, "call");
      }
    });
  }
  return { nodes, edges };
}

function operands(n, pc) {
  const [op, A, B, C, Bx, sBx] = n.ch.code[pc], u = USES[op];
  const rk = (v, on) => on && v >= 256 ? "K" + (v - 256) : v;
  const out = [];
  if (u.includes("A")) out.push(A);
  if (u === "ABx") out.push((op === 36 ? "F" : "K") + Bx);
  else if (u.endsWith("sBx")) out.push("\u2192" + (pc + 1 + sBx));
  else {
    if (u.includes("B")) out.push(rk(B, RK_B.has(op)));
    if (u.includes("C")) out.push(rk(C, RK_C.has(op)));
  }
  return out.join(" ");
}

function annotate(n, pc) {
  const [op, A, B, C, Bx, sBx] = n.ch.code[pc], K = n.ch.k;
  const kv = i => fmtK(K[i]), rk = v => v >= 256 ? kv(v - 256) : "R" + v;
  const up = i => n.ch.upvals[i] || "up" + i;
  if (n.pseudo.has(pc)) return op === 0 ? "captures R" + B : "captures upvalue " + B;
  const arith = "+-*/%^";
  switch (op) {
    case 0: return `R${A} = R${B}`;
    case 1: return `R${A} = ${kv(Bx)}`;
    case 2: return `R${A} = ${B ? "true" : "false"}` + (C ? ", skip next" : "");
    case 3: return `R${A}..R${B} = nil`;
    case 4: return `R${A} = upvalue ${up(B)}`;
    case 5: return `R${A} = _G[${kv(Bx)}]`;
    case 6: return `R${A} = R${B}[${rk(C)}]`;
    case 7: return `_G[${kv(Bx)}] = R${A}`;
    case 8: return `upvalue ${up(B)} = R${A}`;
    case 9: return `R${A}[${rk(B)}] = ${rk(C)}`;
    case 10: return `R${A} = {}`;
    case 11: return `R${A+1} = R${B}; R${A} = R${B}[${rk(C)}]`;
    case 18: return `R${A} = -R${B}`;
    case 19: return `R${A} = not R${B}`;
    case 20: return `R${A} = #R${B}`;
    case 21: return `R${A} = R${B} .. ... .. R${C}`;
    case 22: return `jump to ${pc + 1 + sBx}`;
    case 23: case 24: case 25:
      return `skip next if (${rk(B)} ${["==", "<", "<="][op - 23]} ${rk(C)}) is ${A ? "false" : "true"}`;
    case 26: return `skip next if R${A} is ${C ? "falsy" : "truthy"}`;
    case 27: return `R${A} = R${B} if ${C ? "truthy" : "falsy"}, else skip next`;
    case 28: case 29: return `${op === 29 ? "tail-call " : ""}R${A}(${B === 0 ? "..." : (B - 1) + " args"}) returns ${C === 0 ? "multiple" : C - 1}`;
    case 30: return B === 0 ? `return R${A}...` : `return ${B - 1} value${B === 2 ? "" : "s"}`;
    case 31: return `R${A} += step, loop to ${pc + 1 + sBx} while in range`;
    case 32: return `init loop, jump to ${pc + 1 + sBx}`;
    case 33: return `generic for: call R${A}, ${C} variable${C === 1 ? "" : "s"}`;
    case 34: return `R${A}[batch ${C}] = R${A+1}..${B === 0 ? "top" : "R" + (A + B)}`;
    case 35: return `close upvalues from R${A}`;
    case 36: { const c = n.kids[Bx]; return `R${A} = closure ${c ? c.label : "F" + Bx}`; }
    case 37: return `R${A}.. = ...`;
    default: if (op >= 12 && op <= 17) return `R${A} = ${rk(B)} ${arith[op - 12]} ${rk(C)}`;
  }
  return "";
}
)DJS";
