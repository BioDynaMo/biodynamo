---
title: "Adding documentation webpage"
date: "2019-11-11"
path: "/docs/userguide/add_page/"
meta_title: "BioDynaMo Dev Guide"
meta_description: "Information about adding documentation to the BioDynaMo website"
toc: true
image: ""
sidebar: "userguide"
keywords:
  -website
  -page
  -add
---

Adding your own documentation as part of the BioDynaMo website is very easy.
Besides the requirement of writing your page in compliance with the Markdown
format, there is one more step involved. In order to index the page properly
(e.g. for our search bar), you need to prepend something called a "front matter"
on the top of your page, with the following structure:

```
title: "The title of your article"
date: "2019-11-11"
path: "/docs/userguide/your_url_extension/"
meta_title: "The title of your article"
meta_description: "A brief description of your article"
toc: true
sidebar: "userguide"
keywords:
  -some
  -keywords
  -related
  -to
  -your
  -article
---
```

Please have a look at existing Markdown pages in our repository for examples.

Once you have finalized writing your page in Markdown format, you need to add an
item in the menu bar that refers to your page. This is down in the
userguide.yaml file. Under the right group you can add your page as follows.

```
- group: Documentation
    items:
      - title: The title of your article
        link: /docs/userguide/your_url_extension/
```
