// ==UserScript==
// @name         Explain XKCD shortcut
// @namespace    http://tampermonkey.net/
// @version      0.1
// @description  Shows link to Explain XKCD on xkcd.com
// @author       michaelfm1211
// @match        https://xkcd.com/*
// @icon         data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw==
// @grant        none
// ==/UserScript==

(function() {
    'use strict';

    const ctitle = document.getElementById('ctitle');
    const url = `https://www.explainxkcd.com/wiki/index.php/${location.href.split('/')[3]}`
    ctitle.insertAdjacentHTML('beforebegin', `
<div>
    <span id="ctitle">${ctitle.textContent}</span>
    <a href="${url}" target="_blank" style="font-size: 12px;">explain</a>
</div>`);
    ctitle.remove();
})();