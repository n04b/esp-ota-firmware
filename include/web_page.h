#pragma once

const char INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>ESP Firmware Updater</title>
  <style>
    :root{
      --bg-1:#f4f7fb;
      --bg-2:#e7eef8;
      --card:#ffffffcc;
      --text:#142033;
      --muted:#60708a;
      --line:#d7dfeb;
      --accent:#1677ff;
      --accent-2:#38bdf8;
      --success:#16a34a;
      --error:#dc2626;
      --shadow:0 24px 60px rgba(19,33,68,.14);
      --radius:24px;
    }
    *{box-sizing:border-box}
    body{
      margin:0;
      min-height:100vh;
      font-family:Arial,Helvetica,sans-serif;
      color:var(--text);
      background:
        radial-gradient(circle at top left, rgba(22,119,255,.18), transparent 34%),
        radial-gradient(circle at bottom right, rgba(56,189,248,.18), transparent 32%),
        linear-gradient(160deg,var(--bg-1),var(--bg-2));
      display:grid;
      place-items:center;
      padding:24px;
    }
    .card{
      width:min(100%,560px);
      background:var(--card);
      backdrop-filter:blur(16px);
      border:1px solid rgba(255,255,255,.7);
      border-radius:var(--radius);
      box-shadow:var(--shadow);
      padding:32px;
    }
    h1{
      margin:0 0 10px;
      font-size:clamp(28px,4vw,38px);
      line-height:1.05;
      letter-spacing:-.03em;
    }
    .sub{
      margin:0 0 24px;
      color:var(--muted);
      line-height:1.55;
      font-size:15px;
    }
    .meta{
      display:grid;
      gap:10px;
      margin-bottom:24px;
      padding:16px 18px;
      border-radius:18px;
      background:#f8fbff;
      border:1px solid var(--line);
    }
    .meta strong{font-size:13px;color:var(--muted);display:block;margin-bottom:4px}
    .meta span{font-size:15px;word-break:break-word}
    .dropzone{
      position:relative;
      border:2px dashed #bfd3ef;
      border-radius:22px;
      background:linear-gradient(180deg,rgba(255,255,255,.86),rgba(245,249,255,.96));
      padding:28px 22px;
      text-align:center;
      transition:border-color .2s ease,transform .2s ease,box-shadow .2s ease,background .2s ease;
      box-shadow:inset 0 1px 0 rgba(255,255,255,.7);
    }
    .dropzone.dragover{
      border-color:var(--accent);
      transform:translateY(-2px);
      box-shadow:0 14px 30px rgba(22,119,255,.12);
      background:linear-gradient(180deg,rgba(255,255,255,.98),rgba(235,244,255,1));
    }
    .dropzone h2{
      margin:0 0 8px;
      font-size:20px;
    }
    .dropzone p{
      margin:0;
      color:var(--muted);
      line-height:1.55;
      font-size:15px;
    }
    .actions{
      margin-top:20px;
      display:flex;
      justify-content:center;
    }
    .button{
      appearance:none;
      border:0;
      border-radius:999px;
      padding:14px 22px;
      font-size:15px;
      font-weight:700;
      color:#fff;
      background:linear-gradient(135deg,var(--accent),var(--accent-2));
      box-shadow:0 14px 28px rgba(22,119,255,.22);
      cursor:pointer;
      transition:transform .18s ease,box-shadow .18s ease,filter .18s ease;
    }
    .button:hover{transform:translateY(-1px);box-shadow:0 18px 34px rgba(22,119,255,.28);filter:saturate(1.08)}
    .button:disabled{
      cursor:not-allowed;
      transform:none;
      box-shadow:none;
      opacity:.6;
    }
    input[type=file]{display:none}
    .progress-wrap{
      margin-top:22px;
      padding:14px;
      border-radius:18px;
      background:#f8fbff;
      border:1px solid var(--line);
    }
    .progress-head{
      display:flex;
      align-items:center;
      justify-content:space-between;
      gap:12px;
      margin-bottom:10px;
      font-size:14px;
      color:var(--muted);
    }
    .progress{
      width:100%;
      height:12px;
      overflow:hidden;
      border-radius:999px;
      background:#dfe9f7;
    }
    .bar{
      width:0%;
      height:100%;
      border-radius:999px;
      background:linear-gradient(90deg,var(--accent),var(--accent-2));
      transition:width .2s ease;
      position:relative;
    }
    .bar::after{
      content:"";
      position:absolute;
      inset:0;
      background:linear-gradient(90deg,transparent,rgba(255,255,255,.55),transparent);
      animation:shine 1.4s linear infinite;
    }
    .status{
      margin-top:14px;
      min-height:24px;
      font-size:15px;
      font-weight:600;
    }
    .status.muted{color:var(--muted)}
    .status.success{color:var(--success)}
    .status.error{color:var(--error)}
    .file-name{
      margin-top:14px;
      color:var(--muted);
      font-size:14px;
      word-break:break-word;
    }
    @keyframes shine{
      from{transform:translateX(-100%)}
      to{transform:translateX(100%)}
    }
    @media (max-width:640px){
      .card{padding:24px}
      .dropzone{padding:22px 18px}
      .button{width:100%}
      .actions{display:block}
    }
  </style>
</head>
<body>
  <main class="card">
    <h1>ESP Firmware Updater</h1>
    <p class="sub">Upload a new firmware binary directly from your browser. Drag and drop a <code>.bin</code> file or choose it manually, and the update starts automatically.</p>

    <section class="meta">
      <div>
        <strong>Device IP</strong>
        <span>{{IP}}</span>
      </div>
      <div>
        <strong>mDNS</strong>
        <span><a href="http://{{HOST}}.local" target="_blank" rel="noreferrer">http://{{HOST}}.local</a></span>
      </div>
    </section>

    <section class="dropzone" id="dropzone">
      <h2>Drop firmware here</h2>
      <p>Release the file to start OTA upload instantly.</p>
      <div class="actions">
        <button class="button" id="pickButton" type="button">Choose Firmware</button>
      </div>
      <input id="fileInput" type="file" accept=".bin,.bin.gz,application/octet-stream">
      <div class="file-name" id="fileName">No file selected</div>
    </section>

    <section class="progress-wrap">
      <div class="progress-head">
        <span>Upload progress</span>
        <span id="percentLabel">0%</span>
      </div>
      <div class="progress" aria-hidden="true">
        <div class="bar" id="progressBar"></div>
      </div>
      <div class="status muted" id="statusText">Waiting for firmware file</div>
    </section>
  </main>

  <script>
    (function(){
      const dropzone = document.getElementById('dropzone');
      const fileInput = document.getElementById('fileInput');
      const pickButton = document.getElementById('pickButton');
      const progressBar = document.getElementById('progressBar');
      const percentLabel = document.getElementById('percentLabel');
      const statusText = document.getElementById('statusText');
      const fileName = document.getElementById('fileName');
      let busy = false;

      function setStatus(text, cls) {
        statusText.textContent = text;
        statusText.className = 'status ' + cls;
      }

      function setProgress(value) {
        const safeValue = Math.max(0, Math.min(100, value));
        progressBar.style.width = safeValue + '%';
        percentLabel.textContent = safeValue + '%';
      }

      function beginUpload(file) {
        if (!file || busy) {
          return;
        }

        busy = true;
        fileName.textContent = 'Selected: ' + file.name;
        setProgress(0);
        setStatus('Uploading...', 'muted');
        pickButton.disabled = true;

        const formData = new FormData();
        formData.append('firmware', file, file.name);

        const xhr = new XMLHttpRequest();
        xhr.open('POST', '/update', true);

        xhr.upload.onprogress = function(event) {
          if (event.lengthComputable) {
            setProgress(Math.round((event.loaded / event.total) * 100));
          }
        };

        xhr.onload = function() {
          busy = false;
          pickButton.disabled = false;

          if (xhr.status === 200) {
            setProgress(100);
            setStatus('Success ✅ Device will reboot in 2 seconds.', 'success');
          } else {
            setStatus('Error ❌ ' + (xhr.responseText || 'Upload failed'), 'error');
          }
        };

        xhr.onerror = function() {
          busy = false;
          pickButton.disabled = false;
          setStatus('Error ❌ Network error during upload', 'error');
        };

        xhr.send(formData);
      }

      function handleFiles(files) {
        if (files && files.length > 0) {
          beginUpload(files[0]);
        }
      }

      pickButton.addEventListener('click', function() {
        fileInput.click();
      });

      fileInput.addEventListener('change', function() {
        handleFiles(fileInput.files);
      });

      ['dragenter', 'dragover'].forEach(function(eventName) {
        dropzone.addEventListener(eventName, function(event) {
          event.preventDefault();
          event.stopPropagation();
          dropzone.classList.add('dragover');
        });
      });

      ['dragleave', 'dragend', 'drop'].forEach(function(eventName) {
        dropzone.addEventListener(eventName, function(event) {
          event.preventDefault();
          event.stopPropagation();
          dropzone.classList.remove('dragover');
        });
      });

      dropzone.addEventListener('drop', function(event) {
        handleFiles(event.dataTransfer.files);
      });
    })();
  </script>
</body>
</html>
)HTML";
