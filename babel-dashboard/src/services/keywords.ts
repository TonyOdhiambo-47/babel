// The dashboard's local copy of what the Babel tower recognises.
//
// The Interpreter of Tongues is authoritative — she's the one
// who decides what a phrase translates to. This file exists so
// the voice panel can colour words *immediately*, before the
// network round-trip to the bridge. It's a visual hint, not a
// source of truth.
//
// We keep the two lists in sync by hand: the canonical Babel
// keywords come from babel.c's keyword table, and the loose
// synonyms come from the_book_of_tongues in the same file.

/** Canonical Babel keywords — rendered in the primary accent. */
export const BABEL_KEYWORDS: ReadonlySet<string> = new Set([
  'let', 'there', 'be', 'a', 'an', 'called', 'that', 'equals', 'begins', 'empty',
  'set', 'change', 'to', 'remember', 'as', 'it', 'our',
  'print', 'say', 'separated', 'by', 'with',
  'if', 'otherwise', 'is', 'not', 'and', 'or',
  'greater', 'less', 'than', 'equal', 'divisible',
  'contains', 'in',
  'for', 'every', 'from', 'while', 'do', 'the', 'following', 'stop',
  'to', 'answer', 'give', 'back', 'of', 'length', 'sum',
  'true', 'false',
  'plus', 'minus', 'times', 'divided', 'modulo', 'mod',
  'number', 'word', 'truth', 'list',
]);

/** Loose synonyms the Interpreter will rewrite — rendered in the
 *  secondary accent with a dotted underline to suggest "this
 *  word gets translated". Kept in sync with babel.c's
 *  the_book_of_tongues. */
export const BABEL_SYNONYMS: ReadonlySet<string> = new Set([
  // creation
  'make', 'create', 'define', 'declare', 'need', 'want', 'give',
  // display
  'show', 'display', 'tell', 'reveal', 'log', 'output', 'spit',
  // computation
  'add', 'total', 'tally', 'accumulate', 'count', 'many',
  // change
  'update', 'modify', 'assign', 'replace',
  // iteration
  'go', 'loop', 'iterate', 'each', 'run', 'step', 'cycle', 'check', 'look', 'through', 'over',
  // condition
  'whenever', 'when', 'case', 'assuming', 'suppose', 'given', 'provided', 'else',
  // comparison
  'above', 'over', 'exceeds', 'more', 'higher', 'bigger', 'larger',
  'below', 'under', 'fewer', 'lower', 'smaller',
  'same', 'identical', 'matches',
  // types
  'integer', 'int', 'float', 'decimal', 'string', 'text', 'boolean', 'bool', 'flag', 'array', 'collection',
  // numbers
  'zero', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine',
  'ten', 'twenty', 'thirty', 'forty', 'fifty', 'hundred', 'thousand', 'million',
]);

/** Filler words the Interpreter quietly removes. Rendered at
 *  30% opacity so you can see they were there but that they
 *  don't matter. */
export const BABEL_FILLERS: ReadonlySet<string> = new Set([
  'okay', 'ok', 'alright', 'basically', 'actually', 'really',
  'just', 'um', 'uh', 'uhh', 'hmm', 'like', 'you', 'know',
  'i', 'guess', 'kind', 'sort', 'mean', 'so', 'yeah', 'please',
  'can', 'could', 'would', 'would', "i'd", "let's",
]);

/** Classify a single word from the live transcript. This is a
 *  fast, local, imperfect pass — it's good enough to paint
 *  colours before the Interpreter's verdict arrives. */
export function classifyWord(raw: string): 'keyword' | 'synonym' | 'filler' | 'number' | 'string' | 'unknown' {
  const clean = raw.toLowerCase().replace(/[^a-z0-9]/g, '');
  if (clean.length === 0) return 'unknown';
  if (/^\d+$/.test(clean)) return 'number';
  if (BABEL_KEYWORDS.has(clean)) return 'keyword';
  if (BABEL_SYNONYMS.has(clean)) return 'synonym';
  if (BABEL_FILLERS.has(clean)) return 'filler';
  return 'unknown';
}
