import test from 'ava';
import { formatCopyText } from "./copybutton_funcs";

const parameters = [
	{
		description: 'empty prompt',
		text: 'hallo',
		prompt: '',
		isRegexp: false,
		onlyCopyPromptLines: true,
		removePrompts: true,
		expected: 'hallo'
	},
	{
		description: 'no prompt in text',
		text: 'hallo',
		prompt: '>>> ',
		isRegexp: false,
		onlyCopyPromptLines: true,
		removePrompts: true,
		expected: 'hallo'
	},
	{
		description: 'with non-regexp python prompt',
		text: `
>>> first
output
>>> second`,
		prompt: '>>> ',
		isRegexp: false,
		onlyCopyPromptLines: true,
		removePrompts: true,
		expected: '\nfirst\nsecond'
	},
	{
		description: 'with non-regexp console prompt',
		text: `
$ first
output
$ second`,
		prompt: '$ ',
		isRegexp: false,
		onlyCopyPromptLines: true,
		removePrompts: true,
		expected: '\nfirst\nsecond'
	},
	{
		description: 'with non-regexp prompt, keep prompt',
		text: `
>>> first
output
>>> second`,
		prompt: '>>> ',
		isRegexp: false,
		onlyCopyPromptLines: true,
		removePrompts: false,
		expected: '\n>>> first\n>>> second'
	},
	{
		description: 'multiline with |, keep prompt',
		text: `
>>> first |
output |
is |
fine
is it?
>>> second`,
		prompt: '>>> ',
		isRegexp: false,
		onlyCopyPromptLines: true,
		removePrompts: false,
		copyEmptyLines: false,
		lineContinuationChar: '|',
		expected: '>>> first |\noutput |\nis |\nfine\n>>> second'
	},
	{
		description: 'multiline with \\, remove prompt',
		text: `
$ datalad download-url http://www.tldp.org/LDP/Bash-Beginners-Guide/Bash-Beginners-Guide.pdf \\
--dataset . \\
-m "add beginners guide on bash" \\
-O books/bash_guide.pdf
		`,
		prompt: '$ ',
		isRegexp: false,
		onlyCopyPromptLines: true,
		removePrompts: true,
		copyEmptyLines: false,
		lineContinuationChar: '\\',
		expected: 'datalad download-url http://www.tldp.org/LDP/Bash-Beginners-Guide/Bash-Beginners-Guide.pdf \\\n--dataset . \\\n-m "add beginners guide on bash" \\\n-O books/bash_guide.pdf'
	},
	{
		description: 'multiline with "HERE-document", remove prompt',
		text: `
$ cat << EOT > notes.txt
One can hear a joke.
And laugh.

EOT
		`,
		prompt: '$ ',
		isRegexp: false,
		onlyCopyPromptLines: true,
		removePrompts: true,
		copyEmptyLines: false,
		hereDocDelim: "EOT",
		expected: 'cat << EOT > notes.txt\nOne can hear a joke.\nAnd laugh.\n\nEOT'
	},
	{
		description: 'with non-regexp prompt, keep lines',
		text: `
>>> first
output
>>> second`,
		prompt: '>>> ',
		isRegexp: false,
		onlyCopyPromptLines: false,
		removePrompts: true,
		expected: '\nfirst\noutput\nsecond'
	},
	{
		description: 'with non-regexp prompt, keep all',
		text: `
>>> first
output
>>> second`,
		prompt: '>>> ',
		isRegexp: false,
		onlyCopyPromptLines: false,
		removePrompts: false,
		expected: '\n>>> first\noutput\n>>> second'
	},
	{
		description: 'with regexp python prompt',
		text: `
>>> first
output
>>> second`,
		prompt: '>>> ',
		isRegexp: true,
		onlyCopyPromptLines: true,
		removePrompts: true,
		expected: '\nfirst\nsecond'
	},
	{
		description: 'with regexp python prompt and empty lines',
		text: `
>>> first
output

>>> second`,
		prompt: '>>> ',
		isRegexp: true,
		onlyCopyPromptLines: true,
		removePrompts: true,
		copyEmptyLines: true,
		expected: '\nfirst\n\nsecond'
	},
	{
		description: 'with regexp python prompt and empty lines, ignore empty lines',
		text: `
>>> first
output

>>> second`,
		prompt: '>>> ',
		isRegexp: true,
		onlyCopyPromptLines: true,
		removePrompts: true,
		copyEmptyLines: false,
		expected: 'first\nsecond'
	},
	{
		description: 'with regexp console prompt',
		text: `
$ first
output
$ second`,
		prompt: '\\$ ',
		isRegexp: true,
		onlyCopyPromptLines: true,
		removePrompts: true,
		expected: '\nfirst\nsecond'
	},
	{
		description: 'with ipython prompt (old and new style) regexp',
		text: `
[1]: first
...: continuation
output
[2]: second
In [3]: jupyter`,
		prompt: '\\[\\d*\\]: |In \\[\\d*\\]: |\\.\\.\\.: ',
		isRegexp: true,
		onlyCopyPromptLines: true,
		removePrompts: true,
		expected: '\nfirst\ncontinuation\nsecond\njupyter',
	},
	{
		description: 'with ipython prompt (old and new style) regexp, keep prompts',
		text: `
[1]: first
...: continuation
output
[2]: second
In [3]: jupyter`,
		prompt: '\\[\\d*\\]: |In \\[\\d*\\]: |\\.\\.\\.: ',
		isRegexp: true,
		onlyCopyPromptLines: true,
		removePrompts: false,
		expected: '\n[1]: first\n...: continuation\n[2]: second\nIn [3]: jupyter',
	},
	{
	/* The following is included to demonstrate an example of a false positive regex test.
	As noted in https://github.com/executablebooks/sphinx-copybutton/issues/86,
	JS RegEx in some cases "fixes" incorrect regexes rather than failing on them.
	*/
		description: 'with ipython prompt regexp (false positive)',
		text: `
[1]: first
...: continuation
output
[2]: second`,
		prompt: '[\d*]: |\.\.\.: ',
		isRegexp: true,
		onlyCopyPromptLines: true,
		removePrompts: true,
		expected: '\nfirst\ncontinuation\nsecond'
	}
]

parameters.forEach((p) => {
	test(p.description, t => {
		const text = formatCopyText(p.text, p.prompt, p.isRegexp, p.onlyCopyPromptLines, p.removePrompts, p.copyEmptyLines, p.lineContinuationChar, p.hereDocDelim);
		t.is(text, p.expected)
	});
})
