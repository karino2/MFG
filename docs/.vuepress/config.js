import { viteBundler } from '@vuepress/bundler-vite'
import { defaultTheme } from '@vuepress/theme-default'
import { defineUserConfig } from 'vuepress'
import Prism from 'prismjs'
import PrismMfg from './prism-mfg.js'

Prism.languages.mfg = PrismMfg.grammar


export default defineUserConfig({
  base: "/MFG/",
  locales: {
    "/ja/": {
      lang: "ja-JP",
      title: "MFGドキュメント",
      descpription: "MFGのドキュメント"
    },
    "/en/": {
      lang: "en-US",
      title: "MFG Document",
      descrption: "MFG Documentation."
    }
  },
  bundler: viteBundler(),
  theme: defaultTheme({
    locales: {
      "/ja/": {
        navbar: [
          { text: 'Home', link: '/ja/' },
          { text: 'Getting Started', link: '/ja/GettingStarted/' },
          { text: 'Reference', link: '/ja/Reference/' },
        ],
      },
      "/en/": {
        navbar: [
          { text: 'Home', link: '/en/' },
          { text: 'Getting Started', link: '/en/GettingStarted/' },
          { text: 'Reference', link: '/en/Reference/' },
        ],
      }
    }
  }),
})
