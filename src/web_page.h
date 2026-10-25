// The Leader web page, embedded in the firmware. It is self-contained (no
// external URLs) because the station's Wi-Fi has no internet.

#ifndef SRC_WEB_PAGE_H_
#define SRC_WEB_PAGE_H_

namespace cw {

inline constexpr char kIndexHtml[] = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>CW Trainer</title>
<style>
:root {
  --bg: #f4f1ea; --card: #ffffff; --ink: #1d1b16; --muted: #6b665c;
  --accent: #5b2d8e; --accent-ink: #ffffff; --line: #ddd6c8;
  --play: #1f7a3a; --stop: #b3261e; --warn: #8a5a00;
}
@media (prefers-color-scheme: dark) {
  :root {
    --bg: #15131a; --card: #211e28; --ink: #f1eef6; --muted: #a39db0;
    --accent: #b48be8; --accent-ink: #15131a; --line: #3a3544;
    --play: #5cc27d; --stop: #ff8a80; --warn: #e0b45a;
  }
}
* { box-sizing: border-box; }
body {
  margin: 0; padding: 16px; background: var(--bg); color: var(--ink);
  font: 17px/1.4 system-ui, -apple-system, "Segoe UI", Roboto, sans-serif;
}
main { max-width: 520px; margin: 0 auto; }
h1 { font-size: 1.3rem; margin: 0 0 12px; }
h2 { font-size: 1.05rem; margin: 0 0 12px; color: var(--muted); }
section {
  background: var(--card); border: 1px solid var(--line);
  border-radius: 14px; padding: 16px; margin-bottom: 16px;
}
#banner {
  display: none; padding: 12px; border-radius: 10px; margin-bottom: 16px;
  background: var(--warn); color: var(--bg); font-weight: 600;
}
#answer {
  min-height: 3.2rem; font-size: 2.4rem; font-weight: 700;
  letter-spacing: .15em; text-align: center; overflow-wrap: anywhere;
}
#answer .word { display: inline-block; margin: 0 .4em; }
#meta {
  display: flex; justify-content: space-between; color: var(--muted);
  font-size: .9rem; margin: 6px 0 12px;
}
#playing { color: var(--play); font-weight: 700; visibility: hidden; }
#coach {
  min-height: 1.6rem; text-align: center; font-size: 1.3rem;
  letter-spacing: .2em; color: var(--accent); font-weight: 700;
}
.grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
button {
  min-height: 56px; border: 0; border-radius: 12px; font: inherit;
  font-weight: 650; background: var(--accent); color: var(--accent-ink);
  cursor: pointer;
}
button:active { transform: translateY(1px); }
button.secondary {
  background: transparent; color: var(--ink); border: 2px solid var(--line);
}
#play { background: var(--play); }
#stop { background: var(--stop); }
label {
  display: grid; grid-template-columns: 1fr auto; align-items: center;
  gap: 4px 12px; margin-bottom: 14px;
}
label output { font-variant-numeric: tabular-nums; color: var(--muted); }
input[type=range] { grid-column: 1 / -1; width: 100%; }
.row { display: flex; gap: 8px; flex-wrap: wrap; margin-bottom: 14px; }
.row button { flex: 1; min-height: 44px; }
.row button.on { outline: 3px solid var(--ink); }
ul { list-style: none; padding: 0; margin: 0 0 12px; }
li {
  display: flex; align-items: center; justify-content: space-between;
  padding: 8px 10px; border-bottom: 1px solid var(--line);
}
li.current { font-weight: 700; color: var(--accent); }
li button.pick {
  flex: 1; min-height: 44px; padding: 0; text-align: left;
  background: transparent; color: inherit; font-weight: inherit;
}
li button.delete {
  min-height: 40px; width: 44px; background: transparent;
  color: var(--stop); border: 1px solid var(--line);
}
form.add { display: flex; gap: 8px; }
form.add input {
  flex: 1; min-width: 0; font: inherit; padding: 10px; border-radius: 10px;
  border: 2px solid var(--line); background: var(--bg); color: var(--ink);
  text-transform: uppercase;
}
form.add button { padding: 0 18px; }
#note { color: var(--warn); min-height: 1.4rem; margin-top: 8px; }
</style>
</head>
<body>
<main>
<h1>CW Trainer</h1>
<div id="banner">Not connected &ndash; join the CW-Trainer Wi-Fi</div>

<section>
  <div id="answer">&nbsp;</div>
  <div id="meta"><span id="position"></span>
    <span id="playing">Playing&hellip;</span></div>
  <div id="coach"></div>
  <div class="grid">
    <button id="play">Play</button>
    <button id="stop">Stop</button>
    <button id="reveal" class="secondary">Reveal answer</button>
    <button id="next" class="secondary">Next message</button>
    <button id="random" class="secondary">Random message</button>
  </div>
</section>

<section>
  <h2>Settings</h2>
  <label>Letter speed (WPM) <output id="wpmOut"></output>
    <input id="wpm" type="range" min="5" max="25" step="1"></label>
  <label>Gap between letters <output id="gapOut"></output>
    <input id="gap" type="range" min="0.5" max="6" step="0.5"></label>
  <label>Pitch <output id="pitchOut"></output>
    <input id="pitch" type="range" min="400" max="4000" step="50"></label>
  <div class="row">
    <button class="secondary" data-pitch="800">800 Hz</button>
    <button class="secondary" data-pitch="2000">2000 Hz</button>
    <button class="secondary" data-pitch="4000">4000 Hz (loudest)</button>
  </div>
  <div class="row" id="volume">
    <button class="secondary" data-loud="1">Loud</button>
    <button class="secondary" data-loud="0">Quiet</button>
  </div>
  <label>Plays per press <output id="repeatsOut"></output>
    <input id="repeats" type="range" min="1" max="3" step="1"></label>
  <div class="row" id="coachMode">
    <button class="secondary" data-coach="0">Coach mode off</button>
    <button class="secondary" data-coach="1">Coach mode on</button>
  </div>
  <button id="save" style="width:100%">Save settings</button>
</section>

<section>
  <h2>Messages</h2>
  <ul id="list"></ul>
  <form class="add" id="addForm">
    <input id="newText" maxlength="20" placeholder="New message"
      autocomplete="off">
    <button>Add</button>
  </form>
  <div id="note"></div>
</section>
</main>

<script>
const $ = (id) => document.getElementById(id);
let state = null;
let form = null;
let listKey = '';
let timer = null;

// Returns the parsed reply, or null (with the banner shown) on failure.
async function post(url, fields) {
  try {
    const response = await fetch(url, {
      method: 'POST', body: new URLSearchParams(fields || {})
    });
    const data = await response.json();
    $('banner').style.display = 'none';
    return data;
  } catch (e) {
    $('banner').style.display = 'block';
    return null;
  }
}

// Shows each word as its own centred block, so wrapped lines stay centred.
// Hidden words show one dot per letter.
function renderAnswer(text, revealed) {
  const answer = $('answer');
  answer.textContent = '';
  text.split(' ').forEach((word) => {
    const span = document.createElement('span');
    span.className = 'word';
    span.textContent =
      revealed ? word : word.split('').map(() => '•').join(' ');
    answer.append(span);
  });
}

function renderStatus() {
  const s = state;
  renderAnswer(s.text, s.revealed);
  $('reveal').textContent = s.revealed ? 'Hide answer' : 'Reveal answer';
  $('position').textContent = 'Message ' + (s.index + 1) + ' of ' + s.count;
  $('playing').style.visibility = s.playing ? 'visible' : 'hidden';
  const showCoach = s.settings.coach && s.playing && s.letter >= 0;
  $('coach').textContent = showCoach ? s.text.slice(0, s.letter + 1) : '';
  const key = s.index + '/' + s.count;
  if (key !== listKey) loadList();
}

function renderForm() {
  $('wpm').value = form.wpm;
  $('wpmOut').textContent = form.wpm;
  $('gap').value = form.gapMs / 1000;
  $('gapOut').textContent = (form.gapMs / 1000).toFixed(1) + ' s';
  $('pitch').value = form.pitch;
  $('pitchOut').textContent = form.pitch + ' Hz';
  $('repeats').value = form.repeats;
  $('repeatsOut').textContent = form.repeats;
  document.querySelectorAll('[data-loud]').forEach((b) => {
    b.classList.toggle('on', (b.dataset.loud === '1') === form.loud);
  });
  document.querySelectorAll('[data-coach]').forEach((b) => {
    b.classList.toggle('on', (b.dataset.coach === '1') === form.coach);
  });
}

function applyStatus(s) {
  state = s;
  if (!form) {
    form = Object.assign({}, s.settings);
    renderForm();
  }
  renderStatus();
}

async function poll() {
  clearTimeout(timer);
  try {
    const response = await fetch('/api/status');
    applyStatus(await response.json());
    $('banner').style.display = 'none';
  } catch (e) {
    $('banner').style.display = 'block';
  }
  timer = setTimeout(poll, state && state.playing ? 300 : 1500);
}

async function command(url) {
  const s = await post(url);
  if (s) applyStatus(s);
  poll();
}

function renderList(data) {
  listKey = data.current + '/' + data.messages.length;
  const list = $('list');
  list.textContent = '';
  data.messages.forEach((text, index) => {
    const item = document.createElement('li');
    if (index === data.current) item.className = 'current';
    const pick = document.createElement('button');
    pick.className = 'pick';
    pick.textContent = text;
    pick.onclick = async () => {
      const s = await post('/api/select', {index: index, text: text});
      if (s) applyStatus(s);
      loadList();
      poll();
    };
    const remove = document.createElement('button');
    remove.className = 'delete';
    remove.textContent = '✕';
    remove.setAttribute('aria-label', 'Delete ' + text);
    remove.onclick = async () => {
      if (!confirm('Delete "' + text + '"?')) return;
      const data = await post('/api/messages/delete',
        {index: index, text: text});
      if (!data) return;
      $('note').textContent = data.ok ? '' : data.note;
      renderList(data);
      poll();
    };
    item.append(pick, remove);
    list.append(item);
  });
}

async function loadList() {
  try {
    const response = await fetch('/api/messages');
    renderList(await response.json());
  } catch (e) {}
}

$('play').onclick = () => command('/api/play');
$('stop').onclick = () => command('/api/stop');
$('reveal').onclick = () => command('/api/reveal');
$('next').onclick = () => command('/api/next');
$('random').onclick = () => command('/api/random');

// Applies a settings edit, ignoring it until settings have loaded.
function edit(change) {
  if (!form) return;
  change();
  renderForm();
}

$('wpm').oninput = (e) => edit(() => { form.wpm = +e.target.value; });
$('gap').oninput = (e) => edit(() => {
  form.gapMs = Math.round(+e.target.value * 1000);
});
$('pitch').oninput = (e) => edit(() => { form.pitch = +e.target.value; });
$('repeats').oninput = (e) => edit(() => {
  form.repeats = +e.target.value;
});
document.querySelectorAll('[data-pitch]').forEach((b) => {
  b.onclick = () => edit(() => { form.pitch = +b.dataset.pitch; });
});
document.querySelectorAll('[data-loud]').forEach((b) => {
  b.onclick = () => edit(() => { form.loud = b.dataset.loud === '1'; });
});
document.querySelectorAll('[data-coach]').forEach((b) => {
  b.onclick = () => edit(() => { form.coach = b.dataset.coach === '1'; });
});

$('save').onclick = async () => {
  if (!form) return;
  const s = await post('/api/settings', {
    wpm: form.wpm, gapMs: form.gapMs, pitch: form.pitch,
    loud: form.loud ? 1 : 0, repeats: form.repeats, coach: form.coach ? 1 : 0
  });
  if (!s) return;
  form = Object.assign({}, s.settings);
  renderForm();
  applyStatus(s);
  $('save').textContent = 'Saved';
  setTimeout(() => { $('save').textContent = 'Save settings'; }, 1500);
};

$('addForm').onsubmit = async (e) => {
  e.preventDefault();
  const text = $('newText').value;
  if (!text.trim()) return;
  const data = await post('/api/messages/add', {text: text});
  if (!data) return;
  $('note').textContent = data.ok ? (data.note || 'Added.') : data.note;
  if (data.ok) $('newText').value = '';
  renderList(data);
  poll();
};

poll();
</script>
</body>
</html>
)HTML";

}  // namespace cw

#endif  // SRC_WEB_PAGE_H_
