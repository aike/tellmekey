/* TellMeKey Web UI */
"use strict";

/* ---- JUCE native bridge (minimal shim over __juce__invoke protocol) ---- */

const pendingPromises = new Map();
let nextPromiseId = 0;

if (window.__JUCE__ && window.__JUCE__.backend) {
  window.__JUCE__.backend.addEventListener("__juce__complete", ({ promiseId, result }) => {
    const entry = pendingPromises.get(promiseId);
    if (entry) {
      pendingPromises.delete(promiseId);
      entry(result);
    }
  });
}

function callNative(name, ...params) {
  return new Promise((resolve) => {
    if (!(window.__JUCE__ && window.__JUCE__.backend)) {
      resolve(null); // ブラウザ単体で開いたときのフォールバック
      return;
    }
    const promiseId = nextPromiseId++;
    pendingPromises.set(promiseId, resolve);
    window.__JUCE__.backend.emitEvent("__juce__invoke", {
      name: name,
      params: params,
      resultId: promiseId,
    });
  });
}

/* ---- state ---- */

const state = {
  keymap: null,        // parsed keymap.json
  currentDawId: "unknown",
  compareDawId: "none",
  customPath: "",
  loadError: false,
  isMac: false,        // mac実行時はCtrl/Alt表記をCmd/Optへ読み替えて表示
};

/* ---- helpers ---- */

const $ = (id) => document.getElementById(id);

function dawDisplayName(dawId) {
  const d = state.keymap && state.keymap.daws && state.keymap.daws[dawId];
  return d && d.displayName ? d.displayName : dawId;
}

function splitShortcut(s) {
  // "+" は区切りだが、単独の "+"・"Num +" のような末尾の "+"・
  // スペース直後の "+" は単一キーの一部として扱う
  const parts = [];
  let buf = "";
  for (let i = 0; i < s.length; i++) {
    const c = s[i];
    const isSeparator = c === "+" && i < s.length - 1 && buf.length > 0 && !buf.endsWith(" ");
    if (isSeparator) {
      parts.push(buf);
      buf = "";
    } else {
      buf += c;
    }
  }
  if (buf.length > 0) parts.push(buf);
  return parts.length > 0 ? parts : [s];
}

const MAC_KEY_MAP = { Ctrl: "Cmd", Alt: "Opt" };

function renderKeys(shortcut) {
  if (!shortcut) return '<span class="none">&mdash;</span>';
  let parts = splitShortcut(shortcut);
  if (state.isMac) parts = parts.map((p) => MAC_KEY_MAP[p] || p);
  return parts.map((p) => `<kbd>${escapeHtml(p)}</kbd>`).join("");
}

function escapeHtml(s) {
  return String(s)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;");
}

/* ---- rendering ---- */

function render() {
  const km = state.keymap;
  const unknown = state.currentDawId === "unknown" || !(km.daws && km.daws[state.currentDawId]);

  // header
  const nameEl = $("currentDawName");
  nameEl.textContent = unknown ? "不明なDAW" : dawDisplayName(state.currentDawId);
  nameEl.classList.toggle("unknown", unknown);

  // warning banner
  const banner = $("warningBanner");
  if (state.loadError) {
    banner.textContent =
      "カスタムJSONの読み込みに失敗しました(文法エラーまたはファイルなし)。デフォルト定義を表示しています。";
    banner.classList.remove("hidden");
  } else {
    banner.classList.add("hidden");
  }

  // unknown DAW: ショートカットは一切表示しない
  $("unknownPanel").classList.toggle("hidden", !unknown);
  $("compareBox").classList.toggle("hidden", unknown);
  const list = $("shortcutList");
  if (unknown) {
    list.innerHTML = "";
    return;
  }

  renderCompareSelect();
  renderList();
}

function renderCompareSelect() {
  const sel = $("compareSelect");
  const options = ['<option value="none">なし</option>'];
  for (const dawId of Object.keys(state.keymap.daws)) {
    if (dawId === state.currentDawId) continue;
    const selected = dawId === state.compareDawId ? " selected" : "";
    options.push(`<option value="${escapeHtml(dawId)}"${selected}>${escapeHtml(dawDisplayName(dawId))}</option>`);
  }
  sel.innerHTML = options.join("");
  sel.value = state.compareDawId && state.compareDawId !== state.currentDawId ? state.compareDawId : "none";
}

function renderList() {
  const km = state.keymap;
  const compare = state.compareDawId !== "none" && km.daws[state.compareDawId] ? state.compareDawId : null;

  const categories = Array.isArray(km.categories) ? km.categories : [];
  const knownCatIds = new Set(categories.map((c) => c.id));
  const groups = categories.map((c) => ({ id: c.id, label: c.label, ops: [] }));
  const misc = { id: "_misc", label: "その他", ops: [] };

  for (const op of km.operations || []) {
    const g = knownCatIds.has(op.category) ? groups.find((x) => x.id === op.category) : misc;
    g.ops.push(op);
  }
  if (misc.ops.length > 0) groups.push(misc);

  const html = [];
  for (const g of groups) {
    if (g.ops.length === 0) continue;
    html.push(`<div class="category"><h2>${escapeHtml(g.label)}</h2><table>`);
    html.push("<thead><tr><th></th>");
    html.push(`<th class="key-col">${escapeHtml(dawDisplayName(state.currentDawId))}</th>`);
    if (compare) html.push(`<th class="key-col">${escapeHtml(dawDisplayName(compare))}</th>`);
    html.push("</tr></thead><tbody>");
    for (const op of g.ops) {
      const cur = op.shortcuts ? op.shortcuts[state.currentDawId] : null;
      html.push("<tr>");
      html.push(`<td class="op-name">${escapeHtml(op.label)}</td>`);
      html.push(`<td class="key">${renderKeys(cur)}</td>`);
      if (compare) {
        const cmp = op.shortcuts ? op.shortcuts[compare] : null;
        html.push(`<td class="key compare">${renderKeys(cmp)}</td>`);
      }
      html.push("</tr>");
    }
    html.push("</tbody></table></div>");
  }
  $("shortcutList").innerHTML = html.join("");
}

function renderSettings() {
  $("keymapSource").textContent = state.customPath ? state.customPath : "デフォルト";
}

/* ---- keymap loading ---- */

function applyKeymapResult(result) {
  if (!result) return;
  state.customPath = result.customKeymapPath || "";
  state.loadError = !!result.keymapLoadError;
  try {
    state.keymap = JSON.parse(result.keymapJson);
  } catch (e) {
    // 同梱デフォルトまで壊れていることは通常ないが、念のため
    state.keymap = { daws: {}, categories: [], operations: [] };
    state.loadError = true;
  }
  render();
  renderSettings();
}

/* ---- init & events ---- */

async function init() {
  const data = await callNative("getInitData");
  if (data) {
    state.currentDawId = data.detectedDawId || "unknown";
    state.compareDawId = data.compareDawId || "none";
    state.isMac = !!data.isMac;
    applyKeymapResult(data);
  } else {
    // プラグイン外(通常ブラウザ)で開いた場合の開発用表示
    $("currentDawName").textContent = "(スタンドアロン表示)";
  }

  $("compareSelect").addEventListener("change", (e) => {
    state.compareDawId = e.target.value;
    callNative("setCompareDaw", state.compareDawId);
    renderList();
  });

  $("settingsButton").addEventListener("click", () => {
    renderSettings();
    $("settingsView").classList.remove("hidden");
    $("listView").classList.add("hidden");
    $("dawRow").classList.add("hidden");
  });

  $("backButton").addEventListener("click", () => {
    $("settingsView").classList.add("hidden");
    $("listView").classList.remove("hidden");
    $("dawRow").classList.remove("hidden");
  });

  $("chooseFileButton").addEventListener("click", async () => {
    applyKeymapResult(await callNative("chooseKeymapFile"));
    renderSettings();
  });

  $("clearFileButton").addEventListener("click", async () => {
    applyKeymapResult(await callNative("clearKeymapFile"));
    renderSettings();
  });

  $("reloadButton").addEventListener("click", async () => {
    applyKeymapResult(await callNative("reloadKeymap"));
    renderSettings();
  });
}

document.addEventListener("DOMContentLoaded", init);
