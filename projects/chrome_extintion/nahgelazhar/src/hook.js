(function () {
  'use strict';
  if (window.__nahjHook) return;
  window.__nahjHook = true;

  const RE = /\/Course\/tree-course-with-progress/i;
  let last = null;
  const post = (type, payload) => window.postMessage({ source: 'nahj-hook', type, payload }, '*');
  const origFetch = window.fetch;

  if (typeof origFetch === 'function') {
    window.fetch = function (input, init) {
      const p = origFetch.apply(this, arguments);
      try {
        const url = typeof input === 'string' ? input : (input && input.url) || String(input);
        if (RE.test(url)) {
          const headers = {};
          const h = (init && init.headers) || (input && input.headers);
          if (h) {
            if (Array.isArray(h)) h.forEach(([k, v]) => { headers[k] = v; });
            else if (typeof h.forEach === 'function') h.forEach((v, k) => { headers[k] = v; });
            else Object.assign(headers, h);
          }
          last = { url, headers };
          p.then(r => r.clone().json()).then(j => post('tree', j)).catch(() => { });
        }
      } catch (_) { }
      return p;
    };
  }

  const X = XMLHttpRequest.prototype, open = X.open, send = X.send, setH = X.setRequestHeader;
  X.open = function (m, u) { this.__nahjUrl = String(u); this.__nahjH = {}; return open.apply(this, arguments); };
  X.setRequestHeader = function (k, v) { try { (this.__nahjH = this.__nahjH || {})[k] = v; } catch (_) { } return setH.apply(this, arguments); };
  X.send = function () {
    if (RE.test(this.__nahjUrl || '')) {
      last = { url: this.__nahjUrl, headers: this.__nahjH || {} };
      this.addEventListener('load', () => {
        try {
          const t = this.responseType;
          const j = t === 'json' ? this.response : JSON.parse(t === '' || t === 'text' ? this.responseText : '');
          if (j) post('tree', j);
        } catch (_) { }
      });
    }
    return send.apply(this, arguments);
  };

  window.addEventListener('message', e => {
    if (e.source !== window || !e.data || e.data.source !== 'nahj-ext' || e.data.type !== 'refetch') return;
    if (!last) { post('refetch-failed'); return; }
    const url = last.url.replace(/([?&]_ts=)\d+/, '$1' + Date.now());
    origFetch(url, { headers: last.headers, credentials: 'include' })
      .then(r => r.json()).then(j => post('tree', j)).catch(() => post('refetch-failed'));
  });
})();