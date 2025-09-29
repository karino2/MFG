export default {
  id: 'mfg',
  grammar: {
     'comment': {
    pattern: /(^|[^\\])#.*/,
    lookbehind: true,
    greedy: true,
  },
	'string': {
		pattern: /(^|[^\\])"(?:\\.|[^\\"\r\n])*"(?!\s*:)/,
		lookbehind: true,
		greedy: true
	},
  'tensor-name': {
    pattern: /((?:^|\s)def[ \t]+)[a-zA-Z_]\w*(?=\s*\|)/g,
    lookbehind: true,
    alias: 'class-name'
  },  
  'function': {
    pattern: /((?:^|\s)fn[ \t]+)[a-zA-Z_]\w*(?=\s*\|)/g,
    lookbehind: true,
  },
  'attribute': {
    pattern: /@\w+/,
    alias: 'atrule',
  },
  'number': /\b(?:[\d_]+(?:\.[\d]+u?)?|0x[a-f0-9_]+u?)\b/i,
  'keyword':
    /\b(?:_(?=\s*:)|by|def|fn|let|mut)\b/,
  'builtin':
			/\b(?:abs|all|any|elif|else|f32|f32v[2-4]|input_u8|i32|i32v[2-4]|ifel|max|min|rsum|reduce|sampler|u8|u8v[2-4]|u32|u32v[2-4]|vars|vec[2-4]|xor)\b/,
  'operator': /[-+*/%=!<>&|^~?]+|\|>|\.\.\.|\.\.</,
  'punctuation': /[{}[\](),.:\\]/,    
} ,
}