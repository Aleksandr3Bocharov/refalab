/*
Copyright (c) 2008—2016, Alexander Konovalov, 2016—2021, BMSTU IU9 Department
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions, excepts library and runtime support library components,
  in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

function makeTOC() {
  let prev_level = 1;
  let root = {
    href: null,
    parent: null,
    children: []
  };
  let parent = root;
  let last = null;

  // Источник вдохновения:
  // https://github.com/isaacs/github/issues/215#issuecomment-456598835
  // Оттуда, в частности, взят querySelectorAll.

  document
    .querySelectorAll('h1 > a, h2 > a, h3 > a, h4 > a, h5 > a')
    .forEach((a) =>
  {
    const h = a.parentNode.tagName;
    const target = a.hash;
    const text = a.parentNode.innerHTML.toString().trim()
      .replace(/<a [^>]*>/g, "")
      .replace(/<\/a>/g, "");
    const level = 1 * h.substr(1);
    const href = `<a href="${target}">${text}</a>`;

    if (! target) {
      return;
    }

    while (level > prev_level) {
      if (! last) {
        last = {
          href: parent.href,
          parent: parent,
          children: [],
        };
        parent.children.push(last);
      }

      parent = last;
      last = null;

      ++prev_level;
    }

    while (level < prev_level) {
      last = parent;
      parent = last.parent;

      --prev_level;
    }

    const node = {
      href: href,
      parent: parent,
      children: [],
    };
    parent.children.push(node);
    last = node;
  });

  while (root.href == null && root.children.length == 1) {
    root = root.children[0];
  }

  if (root.children.length == 0) {
    return;
  }

  let fixup_href = (tree) => {
    if (! tree.href) {
      tree.href = fixup_href(tree.children[0]);
    }

    return tree.href;
  };

  fixup_href(root);

  let makelist = (children) => {
    children = children.map((tree) => (
      `<li>${tree.href}<br>${makelist(tree.children)}</li>`
    )).join('');

    if (children) {
      children = `<ul>${children}</ul>`;
    }

    return children;
  };

  const list = makelist(root.children);

  document.getElementById("toc").innerHTML = `
    <p>
      <b>${makeTOC.localizedHeader}</b>
      <a id="showHideTOC" href="#">[${makeTOC.localizedShow}]</a>
    </p>
    <div id="tocGenerated" hidden=false>${list}</div>`;

  const showHideTOC = document.getElementById("showHideTOC");
  showHideTOC.onclick = () => {
    const toc = document.getElementById("tocGenerated");
    if (toc.hidden) {
      toc.hidden = false;
      showHideTOC.innerText = `[${makeTOC.localizedHide}]`;
    } else {
      toc.hidden = true;
      showHideTOC.innerText = `[${makeTOC.localizedShow}]`;
    }
  };
}

makeTOC.localizedHeader = "Table of Contents";
makeTOC.localizedShow = "Show";
makeTOC.localizedHide = "Hide";

document.body.onload = makeTOC;
