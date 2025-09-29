import { viteBundler } from '@vuepress/bundler-vite'
import { defaultTheme } from '@vuepress/theme-default'
import { defineUserConfig } from 'vuepress'
import Prism from 'prismjs'
import PrismMfg from './prism-mfg.js'

Prism.languages.mfg = PrismMfg.grammar


export default defineUserConfig({
  bundler: viteBundler(),
  theme: defaultTheme(),
  base: "/MFG/",
  markdown: {
    extendMarkdown: md => {
      const org_highlight = md.options.highlight
      md.options.highlight =  (code, lang) => {
        if (lang === "mfg")
          return Prism.highlight(code, Prism.languages[lang], lang)
        else
          return org_highlight(code, lang)
      }
    }
  }
})
