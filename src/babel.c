/*
 * BABEL — a tower built of words
 *
 * In the beginning, there was noise. An endless stream of letters
 * with no meaning, no structure, no hope of being understood.
 * The people spoke, but no one could parse what they said.
 *
 * Then came the Lexer.
 * After her came the Parser.
 * After him came the Evaluator.
 *
 * Together they built a tower out of words. The tower was a
 * language. The language is the one you are reading now.
 *
 * This file is their story. It is also the tower itself.
 * Read it as code, and a compiler runs. Read it as prose, and
 * a fable unfolds. Both readings are correct. That was the
 * point of the tower all along.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <setjmp.h>
#include <unistd.h>

/* ================================================================
 * THE MATERIALS
 *
 * Every tower needs raw stone. Ours is bounded by these limits —
 * the most words a single manuscript may carry, the most rooms a
 * single tower may hold, the most names the registry may know.
 * The builders chose generous bounds. The tower would not run
 * out of sky.
 * ================================================================ */

#define THE_LIMIT_OF_WORDS   16384
#define THE_LIMIT_OF_ROOMS   16384
#define THE_LIMIT_OF_THINGS  (1 << 20)
#define THE_LIMIT_OF_NAMES   512
#define THE_LIMIT_OF_TEXT    256
#define THE_LIMIT_OF_LIST    1024
#define THE_LIMIT_OF_BLOCK   512
#define THE_LIMIT_OF_PARAMS  8

/* The signal a return statement raises as it climbs the tower
 * looking for the function call that summoned it. */
static jmp_buf the_homeward_road;
static struct the_thing *the_answer_carried_home;

/* ================================================================
 * CHAPTER ONE: THE LEXER
 *
 * The Lexer was the first builder. Her gift was simple but
 * essential: she could look at the raw noise of speech and find
 * the words hidden inside it. Where others saw a wall of letters,
 * she saw boundaries. Spaces. Periods. The small silences
 * between thoughts.
 *
 * She did not understand what the words meant — that would come
 * later, with the Parser and the Evaluator. Her task was only
 * to recognize that a word had begun, and that a word had ended,
 * and to give each one its proper name.
 * ================================================================ */

typedef enum {
    /* The grammar of creation */
    WORD_LET,           /* a declaration of intent          */
    WORD_THERE,         /* an assertion of existence        */
    WORD_BE,            /* the act of bringing forth        */
    WORD_A,             /* the singular, the specific       */
    WORD_CALLED,        /* the naming, the christening      */
    WORD_THAT,          /* the reference back, the memory   */
    WORD_BEGINS,        /* the origin point                 */
    WORD_EQUALS,        /* the assertion of identity        */

    /* The machinery of repetition */
    WORD_FOR,           /* the promise of repetition        */
    WORD_EVERY,         /* the scope of ambition            */
    WORD_FROM,          /* the starting place               */
    WORD_TO,            /* the destination                  */
    WORD_DO,            /* the commitment to action         */
    WORD_THE,           /* the specific, the known          */
    WORD_FOLLOWING,     /* what comes next                  */
    WORD_WHILE,         /* the persistence of condition     */

    /* The architecture of choice */
    WORD_IF,            /* the fork in the road             */
    WORD_OTHERWISE,     /* the path not taken               */
    WORD_IS,            /* the question of being            */
    WORD_NOT,           /* the negation, the denial         */
    WORD_INDEED,        /* the warm confirmation            */

    /* The voices of the tower */
    WORD_PRINT,         /* the desire to be heard           */
    WORD_SAY,           /* the simpler desire to speak      */

    /* The kinds of things */
    WORD_NUMBER,        /* the kind that counts             */
    WORD_LIST,          /* the kind that gathers            */
    WORD_TRUTH,         /* the kind that judges             */
    WORD_WORD_KIND,     /* the kind that speaks             */

    /* The verbs of memory */
    WORD_REMEMBER,      /* the act of preservation          */
    WORD_AS,            /* the small word of binding        */
    WORD_SET,           /* the act of change                */
    WORD_CHANGE,        /* the admission that change comes  */
    WORD_STOP,          /* the mercy of an early ending     */

    /* The arithmetic of building */
    WORD_PLUS,          /* combination                      */
    WORD_MINUS,         /* separation                       */
    WORD_TIMES,         /* multiplication of effort         */
    WORD_DIVIDED,       /* the splitting of wholes          */
    WORD_BY,            /* the agent of division            */
    WORD_MODULO,        /* what remains after division      */

    /* The judgments */
    WORD_GREATER,       /* the comparison upward            */
    WORD_LESS,          /* the comparison downward          */
    WORD_THAN,          /* the object of comparison         */
    WORD_DIVISIBLE,     /* the question of clean division   */
    WORD_CONTAINS,      /* the question of belonging        */
    WORD_IN,            /* the small word of walking-through */

    /* The measures */
    WORD_EMPTY,         /* the void before creation         */
    WORD_LENGTH,        /* the measure of what was built    */
    WORD_OF,            /* the genitive — belonging-to      */
    WORD_SUM,           /* the weight of accumulation       */
    WORD_SEPARATED,     /* the spaces between               */

    /* The returns */
    WORD_ANSWER,        /* what the tower was built to find */
    WORD_GIVE,          /* the act of returning             */
    WORD_BACK,          /* the direction of return          */

    /* The conjunction and the with */
    WORD_AND,           /* the joining word                 */
    WORD_WITH,          /* the carrying-along word          */

    /* The function-definition opener */
    WORD_TO_DEFINE,     /* "To" at the start of a recipe    */

    /* The most human words */
    WORD_IT,            /* the pronoun, the small mirror    */
    WORD_OUR,           /* the shared, the communal         */

    /* The literal words and atoms */
    WORD_TRUE,          /* the brightest truth              */
    WORD_FALSE,         /* the gentlest denial              */
    WORD_NOTHING,       /* the void with a name             */
    WORD_STRING_LIT,    /* the raw voice of the speaker     */
    WORD_NUMBER_LIT,    /* the raw count                    */
    WORD_NAME_WORD,     /* a word that is not yet known —
                         * perhaps a name being born         */

    /* The marks of punctuation */
    WORD_PERIOD,        /* the end of a thought             */
    WORD_COMMA,         /* the pause                        */
    WORD_COLON,         /* the opening of what follows      */

    /* The shape of the tower */
    WORD_INDENT,        /* deeper into the tower            */
    WORD_DEDENT,        /* back toward the light            */
    WORD_NEWLINE,       /* the breath between lines         */

    WORD_END_OF_SPEECH  /* silence                          */
} the_kinds_of_words;

/* A word once found is kept in this small chest. It remembers
 * what kind of word it is, what letters made it up, and where
 * in the manuscript it was found, so that errors can point home. */
typedef struct {
    the_kinds_of_words what_kind;
    char               the_letters[THE_LIMIT_OF_TEXT];
    double             its_count;
    int                its_line;
    int                its_column;
} the_word;

/* The Lexer keeps her work in a long row of small chests. */
static the_word the_words_she_found[THE_LIMIT_OF_WORDS];
static int      how_many_words_so_far;

/* The Lexer's bag — the small things she carries while she works. */
typedef struct {
    const char *the_speech;
    int         where_she_stands;
    int         the_line_number;
    int         the_column;
    int         depths_remembered[64];
    int         how_deep_she_has_gone;
    int         at_start_of_line;
} the_lexer;

/* When the Interpreter of Tongues (Chapter Eight) is testing a
 * sentence to see if it is already valid Babel, she does not want
 * the tower to stop building if it isn't — she just wants to know
 * whether it would have stopped. These two little fields let her
 * ask. When "the_tower_is_being_patient" is set, a would-be
 * apology quietly longjmps back to her instead of calling exit().
 * The Interpreter always remembers to turn patience off again
 * before the real tower starts building. */
static int     the_tower_is_being_patient = 0;
static jmp_buf the_patient_jump;
static char    the_last_quiet_apology[512];

/* A long, kindly cry. When something cannot be understood, the
 * tower stops building and explains, in plain words, what went
 * wrong. The voice is meant to sound like a patient teacher,
 * never a scolding one. */
static void the_tower_apologizes(int line, const char *why, ...) {
    va_list args;
    if (the_tower_is_being_patient) {
        /* The Interpreter is listening. We save the apology where
         * she can read it later, and return control to her. */
        va_start(args, why);
        vsnprintf(the_last_quiet_apology, sizeof(the_last_quiet_apology), why, args);
        va_end(args);
        longjmp(the_patient_jump, 1);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "  I'm sorry — something went wrong");
    if (line > 0) fprintf(stderr, " on line %d", line);
    fprintf(stderr, ".\n\n  ");
    va_start(args, why);
    vfprintf(stderr, why, args);
    va_end(args);
    fprintf(stderr, "\n\n");
    exit(1);
}

/* She begins, as all builders must, at the very beginning. */
static void the_lexer_begins(the_lexer *she, const char *the_speech) {
    she->the_speech = the_speech;
    she->where_she_stands = 0;
    she->the_line_number = 1;
    she->the_column = 0;
    she->how_deep_she_has_gone = 0;
    she->depths_remembered[0] = 0;
    she->at_start_of_line = 1;
    how_many_words_so_far = 0;
}

/* What she sees, without taking a step. */
static char what_she_sees(the_lexer *she) {
    return she->the_speech[she->where_she_stands];
}

/* A step forward. She remembers each step in the column count. */
static char she_steps_forward(the_lexer *she) {
    char c = she->the_speech[she->where_she_stands++];
    if (c == '\n') { she->the_line_number++; she->the_column = 0; }
    else            { she->the_column++; }
    return c;
}

/* The end of all speech. */
static int the_speech_has_ended(the_lexer *she) {
    return she->the_speech[she->where_she_stands] == '\0';
}

/* She places a word into the long row of chests, gently. */
static the_word* she_keeps(the_kinds_of_words kind, const char *letters,
                           double count, int line, int col) {
    if (how_many_words_so_far >= THE_LIMIT_OF_WORDS) {
        the_tower_apologizes(line, "The manuscript is longer than the tower can hold.");
    }
    the_word *w = &the_words_she_found[how_many_words_so_far++];
    w->what_kind = kind;
    if (letters) {
        strncpy(w->the_letters, letters, THE_LIMIT_OF_TEXT - 1);
        w->the_letters[THE_LIMIT_OF_TEXT - 1] = '\0';
    } else {
        w->the_letters[0] = '\0';
    }
    w->its_count = count;
    w->its_line = line;
    w->its_column = col;
    return w;
}

/* She learned, slowly, to recognize the small words — the ones
 * that carried the grammar of creation. "Let." "There." "Be."
 * Each one a brick in the tower's foundation. */
typedef struct { const char *the_letters; the_kinds_of_words what_kind; } the_naming;

static the_naming the_known_words[] = {
    {"Let",       WORD_LET},       {"let",       WORD_LET},
    {"There",     WORD_THERE},     {"there",     WORD_THERE},
    {"be",        WORD_BE},        {"Be",        WORD_BE},
    {"a",         WORD_A},         {"A",         WORD_A},
    {"an",        WORD_A},         {"An",        WORD_A},
    {"called",    WORD_CALLED},    {"Called",    WORD_CALLED},
    {"that",      WORD_THAT},      {"That",      WORD_THAT},
    {"begins",    WORD_BEGINS},    {"Begins",    WORD_BEGINS},
    {"equals",    WORD_EQUALS},    {"Equals",    WORD_EQUALS},

    {"For",       WORD_FOR},       {"for",       WORD_FOR},
    {"every",     WORD_EVERY},     {"Every",     WORD_EVERY},
    {"from",      WORD_FROM},      {"From",      WORD_FROM},
    {"to",        WORD_TO},        {"To",        WORD_TO_DEFINE},
    {"do",        WORD_DO},        {"Do",        WORD_DO},
    {"the",       WORD_THE},       {"The",       WORD_THE},
    {"following", WORD_FOLLOWING}, {"Following", WORD_FOLLOWING},
    {"While",     WORD_WHILE},     {"while",     WORD_WHILE},

    {"If",        WORD_IF},        {"if",        WORD_IF},
    {"Otherwise", WORD_OTHERWISE}, {"otherwise", WORD_OTHERWISE},
    {"is",        WORD_IS},        {"Is",        WORD_IS},
    {"not",       WORD_NOT},       {"Not",       WORD_NOT},
    {"indeed",    WORD_INDEED},    {"Indeed",    WORD_INDEED},

    {"Print",     WORD_PRINT},     {"print",     WORD_PRINT},
    {"Say",       WORD_SAY},       {"say",       WORD_SAY},

    {"number",    WORD_NUMBER},    {"Number",    WORD_NUMBER},
    {"list",      WORD_LIST},      {"List",      WORD_LIST},
    {"truth",     WORD_TRUTH},     {"Truth",     WORD_TRUTH},
    {"word",      WORD_WORD_KIND}, {"Word",      WORD_WORD_KIND},

    {"Remember",  WORD_REMEMBER},  {"remember",  WORD_REMEMBER},
    {"as",        WORD_AS},        {"As",        WORD_AS},
    {"Set",       WORD_SET},       {"set",       WORD_SET},
    {"Change",    WORD_CHANGE},    {"change",    WORD_CHANGE},
    {"Stop",      WORD_STOP},      {"stop",      WORD_STOP},

    {"plus",      WORD_PLUS},      {"Plus",      WORD_PLUS},
    {"minus",     WORD_MINUS},     {"Minus",     WORD_MINUS},
    {"times",     WORD_TIMES},     {"Times",     WORD_TIMES},
    {"divided",   WORD_DIVIDED},   {"Divided",   WORD_DIVIDED},
    {"by",        WORD_BY},        {"By",        WORD_BY},
    {"modulo",    WORD_MODULO},    {"Modulo",    WORD_MODULO},

    {"greater",   WORD_GREATER},   {"Greater",   WORD_GREATER},
    {"less",      WORD_LESS},      {"Less",      WORD_LESS},
    {"than",      WORD_THAN},      {"Than",      WORD_THAN},
    {"divisible", WORD_DIVISIBLE}, {"Divisible", WORD_DIVISIBLE},
    {"contains",  WORD_CONTAINS},  {"Contains",  WORD_CONTAINS},
    {"in",        WORD_IN},        {"In",        WORD_IN},

    {"empty",     WORD_EMPTY},     {"Empty",     WORD_EMPTY},
    {"length",    WORD_LENGTH},    {"Length",    WORD_LENGTH},
    {"of",        WORD_OF},        {"Of",        WORD_OF},
    {"sum",       WORD_SUM},       {"Sum",       WORD_SUM},
    {"separated", WORD_SEPARATED}, {"Separated", WORD_SEPARATED},

    {"answer",    WORD_ANSWER},    {"Answer",    WORD_ANSWER},
    {"Give",      WORD_GIVE},      {"give",      WORD_GIVE},
    {"back",      WORD_BACK},      {"Back",      WORD_BACK},

    {"and",       WORD_AND},       {"And",       WORD_AND},
    {"with",      WORD_WITH},      {"With",      WORD_WITH},

    {"it",        WORD_IT},        {"It",        WORD_IT},
    {"our",       WORD_OUR},       {"Our",       WORD_OUR},

    {"true",      WORD_TRUE},      {"True",      WORD_TRUE},
    {"false",     WORD_FALSE},     {"False",     WORD_FALSE},
    {"nothing",   WORD_NOTHING},   {"Nothing",   WORD_NOTHING},
    {NULL,        WORD_NAME_WORD}
};

/* She matched the letters of a word against her list of names.
 * If none was found, she called it a name-word — a word that
 * might one day belong to something. */
static the_kinds_of_words she_recognizes(const char *letters) {
    for (int i = 0; the_known_words[i].the_letters; i++) {
        if (strcmp(the_known_words[i].the_letters, letters) == 0)
            return the_known_words[i].what_kind;
    }
    return WORD_NAME_WORD;
}

/* The Lexer's central labor: she walks the manuscript from end
 * to end, and for each silence-bounded chunk of letters she finds,
 * she gives it a name and places it in her long row of chests.
 * She also notices the indentation of each new line, and emits
 * the small invisible markers — INDENT and DEDENT — that tell
 * the Parser how the tower's floors are stacked. */
static void the_lexer_speaks_through_the_whole_manuscript(the_lexer *she) {
    while (!the_speech_has_ended(she)) {

        /* At the start of a line, she counts the leading silence
         * to learn how deep the next thought lives. */
        if (she->at_start_of_line) {
            int the_depth = 0;
            while (!the_speech_has_ended(she)) {
                char c = what_she_sees(she);
                if (c == ' ')      { the_depth++;     she_steps_forward(she); }
                else if (c == '\t'){ the_depth += 4;  she_steps_forward(she); }
                else break;
            }
            /* Blank lines and comment lines do not change depth. */
            char peek = what_she_sees(she);
            if (peek == '\n' || peek == '\0' || peek == '#') {
                /* skip the blank-or-comment line entirely */
                while (!the_speech_has_ended(she) && what_she_sees(she) != '\n')
                    she_steps_forward(she);
                if (!the_speech_has_ended(she)) she_steps_forward(she);
                continue;
            }

            int top = she->depths_remembered[she->how_deep_she_has_gone];
            if (the_depth > top) {
                if (she->how_deep_she_has_gone + 1 >= (int)(sizeof(she->depths_remembered) / sizeof(she->depths_remembered[0])))
                    the_tower_apologizes(she->the_line_number,
                        "The tower has too many floors at once — "
                        "this line is indented deeper than the builders can reach.");
                she->depths_remembered[++she->how_deep_she_has_gone] = the_depth;
                she_keeps(WORD_INDENT, NULL, 0, she->the_line_number, she->the_column);
            } else {
                while (the_depth < she->depths_remembered[she->how_deep_she_has_gone]) {
                    she->how_deep_she_has_gone--;
                    she_keeps(WORD_DEDENT, NULL, 0, she->the_line_number, she->the_column);
                }
            }
            she->at_start_of_line = 0;
        }

        char c = what_she_sees(she);

        /* Silence between words she simply walked past. */
        if (c == ' ' || c == '\t') { she_steps_forward(she); continue; }

        /* The breath at the end of a line. */
        if (c == '\n') {
            she_keeps(WORD_NEWLINE, NULL, 0, she->the_line_number, she->the_column);
            she_steps_forward(she);
            she->at_start_of_line = 1;
            continue;
        }

        /* A small mark of punctuation. */
        if (c == '.') { she_keeps(WORD_PERIOD, ".", 0, she->the_line_number, she->the_column); she_steps_forward(she); continue; }
        if (c == ',') { she_keeps(WORD_COMMA, ",", 0, she->the_line_number, she->the_column); she_steps_forward(she); continue; }
        if (c == ':') { she_keeps(WORD_COLON, ":", 0, she->the_line_number, she->the_column); she_steps_forward(she); continue; }

        /* The voice of a speaker — anything wrapped in quotation
         * marks belongs to them, exactly as written. */
        if (c == '"') {
            int line = she->the_line_number, col = she->the_column;
            she_steps_forward(she); /* swallow the opening quote */
            char buf[THE_LIMIT_OF_TEXT];
            int n = 0;
            while (!the_speech_has_ended(she) && what_she_sees(she) != '"' && what_she_sees(she) != '\n') {
                if (n >= THE_LIMIT_OF_TEXT - 1)
                    the_tower_apologizes(line, "A spoken phrase is longer than the tower can carry.");
                buf[n++] = she_steps_forward(she);
            }
            if (the_speech_has_ended(she) || what_she_sees(she) != '"')
                the_tower_apologizes(line,
                    "A spoken phrase began with a quotation mark but never ended. "
                    "Every \" must be closed by another \" on the same line.");
            buf[n] = '\0';
            she_steps_forward(she); /* closing quote */
            she_keeps(WORD_STRING_LIT, buf, 0, line, col);
            continue;
        }

        /* A raw count: a sequence of digits, possibly with a single
         * decimal point — but only if a digit follows the point, so
         * that the period at the end of a sentence is not eaten. */
        if (isdigit((unsigned char)c)) {
            int line = she->the_line_number, col = she->the_column;
            char buf[64];
            int n = 0;
            while (!the_speech_has_ended(she) && isdigit((unsigned char)what_she_sees(she))) {
                if (n >= 63) break;
                buf[n++] = she_steps_forward(she);
            }
            if (what_she_sees(she) == '.' &&
                isdigit((unsigned char)she->the_speech[she->where_she_stands + 1])) {
                if (n < 63) buf[n++] = she_steps_forward(she);
                while (!the_speech_has_ended(she) && isdigit((unsigned char)what_she_sees(she))) {
                    if (n >= 63) break;
                    buf[n++] = she_steps_forward(she);
                }
            }
            buf[n] = '\0';
            she_keeps(WORD_NUMBER_LIT, buf, atof(buf), line, col);
            continue;
        }

        /* A word — a run of letters, possibly with apostrophes
         * or underscores in the middle. */
        if (isalpha((unsigned char)c) || c == '_') {
            int line = she->the_line_number, col = she->the_column;
            char buf[THE_LIMIT_OF_TEXT];
            int n = 0;
            while (!the_speech_has_ended(she)) {
                char ch = what_she_sees(she);
                if (isalpha((unsigned char)ch) || isdigit((unsigned char)ch) || ch == '_' || ch == '\'') {
                    if (n >= THE_LIMIT_OF_TEXT - 1) break;
                    buf[n++] = she_steps_forward(she);
                } else break;
            }
            buf[n] = '\0';
            the_kinds_of_words k = she_recognizes(buf);
            she_keeps(k, buf, 0, line, col);
            continue;
        }

        /* Anything else, she gently ignores, taking a single step
         * past it so the manuscript continues to flow. */
        she_steps_forward(she);
    }

    /* As she leaves, she closes any rooms she had opened. */
    while (she->how_deep_she_has_gone > 0) {
        she->how_deep_she_has_gone--;
        she_keeps(WORD_DEDENT, NULL, 0, she->the_line_number, she->the_column);
    }
    she_keeps(WORD_END_OF_SPEECH, NULL, 0, she->the_line_number, she->the_column);
}

/* ================================================================
 * CHAPTER TWO: THE PARSER
 *
 * The Parser was the architect. Where the Lexer found the words,
 * the Parser found the meaning that the words were arranging
 * themselves into. He took her stream of small wooden tokens and
 * built them into rooms — statements — and the rooms into floors
 * — blocks — and the floors into the tower itself.
 *
 * His tower was a tree. Each room a node. Each floor a branch.
 * He called it the Tree of Abstract Meaning, though the builders
 * who came after him simply called it "the plan."
 * ================================================================ */

typedef enum {
    ROOM_NUMBER,
    ROOM_STRING,
    ROOM_BOOLEAN,
    ROOM_NOTHING,
    ROOM_VARIABLE,
    ROOM_BINARY_OP,
    ROOM_NEGATION,
    ROOM_LENGTH,
    ROOM_SUM,
    ROOM_LIST_ACCESS,

    ROOM_DECLARATION,
    ROOM_ASSIGNMENT,
    ROOM_REMEMBER,
    ROOM_PRINT,
    ROOM_SAY,
    ROOM_CONDITIONAL,
    ROOM_FOR_LOOP,
    ROOM_WHILE_LOOP,
    ROOM_FUNCTION_DEF,
    ROOM_FUNCTION_CALL,
    ROOM_RETURN,
    ROOM_STOP,
    ROOM_BLOCK
} the_kinds_of_rooms;

typedef struct the_room the_room;

struct the_room {
    the_kinds_of_rooms what_kind;
    int                which_line;

    /* The contents of every kind of room. Many fields are unused
     * for any given kind, but the union of all of them is small
     * enough that the Parser does not bother to be clever. */
    double the_count;                  /* number literal               */
    char   the_text[THE_LIMIT_OF_TEXT];/* string / name / op symbol    */
    int    the_truth;                  /* boolean literal              */

    the_room *the_left;                /* left side of binary, etc.    */
    the_room *the_right;               /* right side of binary         */

    /* For declaration */
    char the_type[32];

    /* For for-loops */
    char the_counter_name[THE_LIMIT_OF_TEXT];
    the_room *the_beginning;
    the_room *the_destination;
    /* When is_foreach is 1, the for-loop walks the elements of the
     * list whose name lives in the_text, instead of counting from
     * the_beginning up to the_destination. The counter bound to
     * the_counter_name then holds each element in turn. */
    int is_foreach;

    /* For function definitions and calls */
    char  the_recipe_name[THE_LIMIT_OF_TEXT];
    char  the_ingredients[THE_LIMIT_OF_PARAMS][THE_LIMIT_OF_TEXT];
    int   how_many_ingredients;
    the_room *the_arguments[THE_LIMIT_OF_PARAMS];
    int   how_many_arguments;

    /* For prints */
    the_room *the_separator;

    /* For blocks */
    the_room *the_rooms_within[THE_LIMIT_OF_BLOCK];
    int       how_many_rooms_within;
};

/* The Parser kept his rooms in a long row, exactly as the Lexer
 * had kept her words. He took from this row whenever he needed
 * a new room, and he never gave one back, because the tower
 * was meant to last only as long as the conversation. */
static the_room the_rooms_pool[THE_LIMIT_OF_ROOMS];
static int      how_many_rooms_built;

static the_room* a_new_room(the_kinds_of_rooms what_kind, int line) {
    if (how_many_rooms_built >= THE_LIMIT_OF_ROOMS)
        the_tower_apologizes(line, "This tower has more rooms than the sky has space for.");
    the_room *r = &the_rooms_pool[how_many_rooms_built++];
    memset(r, 0, sizeof(*r));
    r->what_kind = what_kind;
    r->which_line = line;
    return r;
}

/* The Parser's small bag of tools: where he is in the row of
 * words, and the names he has heard so far in the current scope
 * (so that the next time he hears a name he can recognize it
 * instead of mistaking it for a stranger). */
typedef struct {
    int  where_he_stands;
    char the_known_names[THE_LIMIT_OF_NAMES][THE_LIMIT_OF_TEXT];
    int  is_a_recipe[THE_LIMIT_OF_NAMES];      /* whether the name is a function */
    int  how_many_names;
    char the_last_name[THE_LIMIT_OF_TEXT];     /* for "it" resolution */
} the_parser;

static the_parser the_one_who_builds;

static the_word* what_he_sees(void) {
    return &the_words_she_found[the_one_who_builds.where_he_stands];
}
static the_word* what_he_sees_next(int how_far_ahead) {
    int p = the_one_who_builds.where_he_stands + how_far_ahead;
    if (p >= how_many_words_so_far) p = how_many_words_so_far - 1;
    return &the_words_she_found[p];
}
static the_word* he_takes_a_step(void) {
    the_word *w = what_he_sees();
    if (the_one_who_builds.where_he_stands < how_many_words_so_far - 1)
        the_one_who_builds.where_he_stands++;
    return w;
}
static int he_accepts(the_kinds_of_words kind) {
    if (what_he_sees()->what_kind == kind) { he_takes_a_step(); return 1; }
    return 0;
}
static void he_demands(the_kinds_of_words kind, const char *what_was_expected) {
    the_word *w = what_he_sees();
    if (w->what_kind != kind) {
        the_tower_apologizes(w->its_line,
            "I was expecting %s here, but instead I found \"%s\".\n  "
            "It looks like the sentence was not finished the way I'd hoped.",
            what_was_expected, w->the_letters);
    }
    he_takes_a_step();
}

/* He skips past the soft breaths between lines whenever those
 * breaths are not load-bearing. */
static void he_skips_blank_lines(void) {
    while (what_he_sees()->what_kind == WORD_NEWLINE) he_takes_a_step();
}

/* He remembers a new name in his scope so that future references
 * to it will not surprise him. */
static void he_now_knows(const char *name) {
    if (the_one_who_builds.how_many_names >= THE_LIMIT_OF_NAMES) return;
    /* avoid duplicates so the registry stays slim */
    for (int i = 0; i < the_one_who_builds.how_many_names; i++)
        if (strcmp(the_one_who_builds.the_known_names[i], name) == 0) goto remember_last;
    strncpy(the_one_who_builds.the_known_names[the_one_who_builds.how_many_names],
            name, THE_LIMIT_OF_TEXT - 1);
    the_one_who_builds.is_a_recipe[the_one_who_builds.how_many_names] = 0;
    the_one_who_builds.how_many_names++;
remember_last:
    strncpy(the_one_who_builds.the_last_name, name, THE_LIMIT_OF_TEXT - 1);
}

/* He remembers that a particular name belongs to a recipe rather
 * than to an ordinary value. This lets him later recognize a call
 * even when it appears in the middle of an expression. */
static void he_now_knows_a_recipe(const char *name) {
    he_now_knows(name);
    for (int i = 0; i < the_one_who_builds.how_many_names; i++)
        if (strcmp(the_one_who_builds.the_known_names[i], name) == 0)
            the_one_who_builds.is_a_recipe[i] = 1;
}

static int the_name_is_a_recipe(const char *name) {
    for (int i = 0; i < the_one_who_builds.how_many_names; i++)
        if (strcmp(the_one_who_builds.the_known_names[i], name) == 0)
            return the_one_who_builds.is_a_recipe[i];
    return 0;
}

/* Whether a sequence of words at the current position spells out
 * a name he already knows. He prefers the longest match, because
 * "the distance between stars" should not be mistaken for "the". */
static int he_finds_a_known_name(char out[THE_LIMIT_OF_TEXT]) {
    int best_length = 0;
    out[0] = '\0';
    for (int i = 0; i < the_one_who_builds.how_many_names; i++) {
        const char *known = the_one_who_builds.the_known_names[i];
        /* split known name on spaces */
        char copy[THE_LIMIT_OF_TEXT];
        strncpy(copy, known, THE_LIMIT_OF_TEXT - 1); copy[THE_LIMIT_OF_TEXT - 1] = '\0';
        char *parts[16]; int how_many_parts = 0;
        char *tok = strtok(copy, " ");
        while (tok && how_many_parts < 16) { parts[how_many_parts++] = tok; tok = strtok(NULL, " "); }
        /* attempt to match each part against successive tokens */
        int matches = 1;
        for (int p = 0; p < how_many_parts; p++) {
            the_word *w = what_he_sees_next(p);
            if (w->what_kind != WORD_NAME_WORD) { matches = 0; break; }
            if (strcmp(w->the_letters, parts[p]) != 0) { matches = 0; break; }
        }
        if (matches && how_many_parts > best_length) {
            best_length = how_many_parts;
            strncpy(out, known, THE_LIMIT_OF_TEXT - 1);
        }
    }
    return best_length;
}

/* Forward voices, calling each other from across the file. */
static the_room* the_parser_reads_a_statement(void);
static the_room* the_parser_reads_an_expression(void);
static the_room* the_parser_reads_a_block(void);
static the_room* the_parser_reads_a_condition(void);

/* He reads the smallest thing that can stand alone as a value:
 * a literal, a variable name, a parenthetical, or one of the
 * built-in measures like "the length of X". */
static the_room* the_parser_reads_a_term(void) {
    the_word *w = what_he_sees();
    int line = w->its_line;

    if (w->what_kind == WORD_NUMBER_LIT) {
        the_room *r = a_new_room(ROOM_NUMBER, line);
        r->the_count = w->its_count;
        he_takes_a_step();
        return r;
    }
    if (w->what_kind == WORD_STRING_LIT) {
        the_room *r = a_new_room(ROOM_STRING, line);
        strncpy(r->the_text, w->the_letters, THE_LIMIT_OF_TEXT - 1);
        he_takes_a_step();
        return r;
    }
    if (w->what_kind == WORD_TRUE)  { he_takes_a_step(); the_room *r = a_new_room(ROOM_BOOLEAN, line); r->the_truth = 1; return r; }
    if (w->what_kind == WORD_FALSE) { he_takes_a_step(); the_room *r = a_new_room(ROOM_BOOLEAN, line); r->the_truth = 0; return r; }
    if (w->what_kind == WORD_NOTHING) { he_takes_a_step(); return a_new_room(ROOM_NOTHING, line); }

    /* "the length of X" / "the sum of X" / "the Nth element of X" */
    if (w->what_kind == WORD_THE) {
        he_takes_a_step();
        the_word *next = what_he_sees();
        if (next->what_kind == WORD_LENGTH) {
            he_takes_a_step();
            he_demands(WORD_OF, "the word \"of\"");
            char name[THE_LIMIT_OF_TEXT];
            if (!he_finds_a_known_name(name))
                the_tower_apologizes(line, "I expected the name of a list after \"the length of\".");
            for (int p = 0; name[p]; p++) if (name[p] == ' ') {} ;
            /* advance past name parts */
            int parts = 1; for (const char *q = name; *q; q++) if (*q == ' ') parts++;
            for (int p = 0; p < parts; p++) he_takes_a_step();
            the_room *r = a_new_room(ROOM_LENGTH, line);
            strncpy(r->the_text, name, THE_LIMIT_OF_TEXT - 1);
            return r;
        }
        if (next->what_kind == WORD_SUM) {
            he_takes_a_step();
            he_demands(WORD_OF, "the word \"of\"");
            char name[THE_LIMIT_OF_TEXT];
            if (!he_finds_a_known_name(name))
                the_tower_apologizes(line, "I expected the name of a list after \"the sum of\".");
            int parts = 1; for (const char *q = name; *q; q++) if (*q == ' ') parts++;
            for (int p = 0; p < parts; p++) he_takes_a_step();
            the_room *r = a_new_room(ROOM_SUM, line);
            strncpy(r->the_text, name, THE_LIMIT_OF_TEXT - 1);
            return r;
        }
        the_tower_apologizes(line, "I saw \"the\" but the next word, \"%s\", surprised me.", next->the_letters);
    }

    /* The pronoun "it" — the most human word. It refers to
     * whatever name was most recently spoken. */
    if (w->what_kind == WORD_IT) {
        he_takes_a_step();
        if (the_one_who_builds.the_last_name[0] == '\0')
            the_tower_apologizes(line, "I saw \"it\" but I'm not sure what \"it\" is yet.");
        the_room *r = a_new_room(ROOM_VARIABLE, line);
        strncpy(r->the_text, the_one_who_builds.the_last_name, THE_LIMIT_OF_TEXT - 1);
        return r;
    }

    /* "our X" — a possessive way of saying X. */
    if (w->what_kind == WORD_OUR) {
        he_takes_a_step();
        char name[THE_LIMIT_OF_TEXT];
        if (!he_finds_a_known_name(name))
            the_tower_apologizes(line, "I saw \"our\" but the next word wasn't a name I knew.");
        int parts = 1; for (const char *q = name; *q; q++) if (*q == ' ') parts++;
        for (int p = 0; p < parts; p++) he_takes_a_step();
        the_room *r = a_new_room(ROOM_VARIABLE, line);
        strncpy(r->the_text, name, THE_LIMIT_OF_TEXT - 1);
        strncpy(the_one_who_builds.the_last_name, name, THE_LIMIT_OF_TEXT - 1);
        return r;
    }

    /* A bare name — match the longest known name starting here.
     * If the name belongs to a recipe, parse a call expression. */
    if (w->what_kind == WORD_NAME_WORD) {
        char name[THE_LIMIT_OF_TEXT];
        int parts = he_finds_a_known_name(name);
        if (parts > 0) {
            for (int p = 0; p < parts; p++) he_takes_a_step();
            if (the_name_is_a_recipe(name)) {
                the_room *r = a_new_room(ROOM_FUNCTION_CALL, line);
                strncpy(r->the_recipe_name, name, THE_LIMIT_OF_TEXT - 1);
                if (he_accepts(WORD_WITH)) {
                    while (1) {
                        if (r->how_many_arguments >= THE_LIMIT_OF_PARAMS)
                            the_tower_apologizes(line, "Too many arguments.");
                        r->the_arguments[r->how_many_arguments++] = the_parser_reads_an_expression();
                        if (!he_accepts(WORD_AND)) break;
                    }
                }
                strncpy(the_one_who_builds.the_last_name, name, THE_LIMIT_OF_TEXT - 1);
                return r;
            }
            the_room *r = a_new_room(ROOM_VARIABLE, line);
            strncpy(r->the_text, name, THE_LIMIT_OF_TEXT - 1);
            strncpy(the_one_who_builds.the_last_name, name, THE_LIMIT_OF_TEXT - 1);
            return r;
        }
        the_tower_apologizes(line,
            "I don't know what \"%s\" means yet.\n  "
            "In Babel, you introduce a name with \"Let there be...\" before using it.",
            w->the_letters);
    }

    the_tower_apologizes(line,
        "I was expecting a value here, but I found \"%s\" instead.",
        w->the_letters);
    return NULL; /* never reached */
}

/* Multiplication and division bind tighter than plus and minus. */
static the_room* the_parser_reads_a_factor(void) {
    the_room *left = the_parser_reads_a_term();
    while (1) {
        the_word *w = what_he_sees();
        if (w->what_kind == WORD_TIMES) {
            he_takes_a_step();
            the_room *right = the_parser_reads_a_term();
            the_room *r = a_new_room(ROOM_BINARY_OP, w->its_line);
            strcpy(r->the_text, "*"); r->the_left = left; r->the_right = right; left = r;
        } else if (w->what_kind == WORD_DIVIDED) {
            he_takes_a_step();
            he_demands(WORD_BY, "the word \"by\"");
            the_room *right = the_parser_reads_a_term();
            the_room *r = a_new_room(ROOM_BINARY_OP, w->its_line);
            strcpy(r->the_text, "/"); r->the_left = left; r->the_right = right; left = r;
        } else if (w->what_kind == WORD_MODULO) {
            he_takes_a_step();
            the_room *right = the_parser_reads_a_term();
            the_room *r = a_new_room(ROOM_BINARY_OP, w->its_line);
            strcpy(r->the_text, "%"); r->the_left = left; r->the_right = right; left = r;
        } else break;
    }
    return left;
}

/* Plus and minus, the slower-binding combinators. */
static the_room* the_parser_reads_an_expression(void) {
    the_room *left = the_parser_reads_a_factor();
    while (1) {
        the_word *w = what_he_sees();
        if (w->what_kind == WORD_PLUS) {
            he_takes_a_step();
            the_room *right = the_parser_reads_a_factor();
            the_room *r = a_new_room(ROOM_BINARY_OP, w->its_line);
            strcpy(r->the_text, "+"); r->the_left = left; r->the_right = right; left = r;
        } else if (w->what_kind == WORD_MINUS) {
            he_takes_a_step();
            the_room *right = the_parser_reads_a_factor();
            the_room *r = a_new_room(ROOM_BINARY_OP, w->its_line);
            strcpy(r->the_text, "-"); r->the_left = left; r->the_right = right; left = r;
        } else break;
    }
    return left;
}

/* A condition is a comparison between two values. The Parser
 * recognizes the small handful of comparators Babel knows. */
static the_room* the_parser_reads_a_condition(void) {
    the_room *left = the_parser_reads_an_expression();
    the_word *w = what_he_sees();
    int line = w->its_line;
    const char *op = NULL;
    if (w->what_kind == WORD_IS) {
        he_takes_a_step();
        the_word *next = what_he_sees();
        if (next->what_kind == WORD_GREATER)      { he_takes_a_step(); he_demands(WORD_THAN, "the word \"than\""); op = ">"; }
        else if (next->what_kind == WORD_LESS)    { he_takes_a_step(); he_demands(WORD_THAN, "the word \"than\""); op = "<"; }
        else if (next->what_kind == WORD_DIVISIBLE){ he_takes_a_step(); he_demands(WORD_BY, "the word \"by\""); op = "div"; }
        else if (next->what_kind == WORD_NOT) {
            he_takes_a_step();
            the_word *after = what_he_sees();
            if (after->what_kind == WORD_DIVISIBLE) { he_takes_a_step(); he_demands(WORD_BY, "the word \"by\""); op = "ndiv"; }
            else { op = "neq"; }
        }
        else if (next->what_kind == WORD_EQUALS)  { he_takes_a_step(); op = "=="; }
        else { op = "is"; } /* "X is Y" — equality */
    } else if (w->what_kind == WORD_EQUALS) {
        he_takes_a_step(); op = "==";
    } else if (w->what_kind == WORD_CONTAINS) {
        he_takes_a_step(); op = "contains";
    } else {
        the_tower_apologizes(line, "I expected a comparison here (like \"is greater than\"), but found \"%s\".", w->the_letters);
    }
    the_room *right = the_parser_reads_an_expression();
    the_room *r = a_new_room(ROOM_BINARY_OP, line);
    strncpy(r->the_text, op, sizeof(r->the_text) - 1);
    r->the_left = left; r->the_right = right;
    return r;
}

/* The block reader: he gathers everything between an INDENT and
 * its matching DEDENT into a single room of kind ROOM_BLOCK. */
static the_room* the_parser_reads_a_block(void) {
    he_skips_blank_lines();
    he_demands(WORD_INDENT, "an indented block");
    the_room *block = a_new_room(ROOM_BLOCK, what_he_sees()->its_line);
    while (1) {
        he_skips_blank_lines();
        if (what_he_sees()->what_kind == WORD_DEDENT) { he_takes_a_step(); break; }
        if (what_he_sees()->what_kind == WORD_END_OF_SPEECH) break;
        the_room *s = the_parser_reads_a_statement();
        if (s) {
            if (block->how_many_rooms_within >= THE_LIMIT_OF_BLOCK)
                the_tower_apologizes(s->which_line,
                    "This room has more sentences in it than the tower can hold. "
                    "Consider moving some of them into a recipe of their own.");
            block->the_rooms_within[block->how_many_rooms_within++] = s;
        }
    }
    return block;
}

/* The kind-name reader for declarations. */
static const char* he_reads_a_kind(void) {
    the_word *w = what_he_sees();
    if (he_accepts(WORD_NUMBER))    return "number";
    if (he_accepts(WORD_LIST))      return "list";
    if (he_accepts(WORD_TRUTH))     return "truth";
    if (he_accepts(WORD_WORD_KIND)) return "word";
    if (he_accepts(WORD_NOTHING))   return "nothing";
    the_tower_apologizes(w->its_line, "I expected a kind here (number, list, truth, or word), but found \"%s\".", w->the_letters);
    return "";
}

/* The single largest room in the Parser's house: the statement
 * reader. He looks at the first word of the next sentence and,
 * by it, decides which kind of room he must build. */
static the_room* the_parser_reads_a_statement(void) {
    he_skips_blank_lines();
    the_word *w = what_he_sees();
    int line = w->its_line;

    /* "Let there be a TYPE called \"NAME\" [that begins/equals EXPR]." */
    if (he_accepts(WORD_LET)) {
        he_demands(WORD_THERE, "the word \"there\"");
        he_demands(WORD_BE,    "the word \"be\"");
        he_accepts(WORD_A);
        const char *kind = he_reads_a_kind();
        he_demands(WORD_CALLED, "the word \"called\"");
        the_word *nm = what_he_sees();
        if (nm->what_kind != WORD_STRING_LIT)
            the_tower_apologizes(line, "After \"called\" I need a name in quotes, like \"primes\".");
        char name[THE_LIMIT_OF_TEXT];
        strncpy(name, nm->the_letters, THE_LIMIT_OF_TEXT - 1);
        he_takes_a_step();

        the_room *init = NULL;
        if (he_accepts(WORD_THAT)) {
            if (he_accepts(WORD_BEGINS)) {
                if (he_accepts(WORD_EMPTY)) {
                    init = a_new_room(ROOM_NOTHING, line);
                } else {
                    init = the_parser_reads_an_expression();
                }
            } else if (he_accepts(WORD_EQUALS)) {
                init = the_parser_reads_an_expression();
            } else {
                the_tower_apologizes(line, "After \"that\" I expected \"begins\" or \"equals\".");
            }
        }
        he_demands(WORD_PERIOD, "a period at the end of the declaration");
        the_room *r = a_new_room(ROOM_DECLARATION, line);
        strncpy(r->the_text, name, THE_LIMIT_OF_TEXT - 1);
        strncpy(r->the_type, kind, sizeof(r->the_type) - 1);
        r->the_left = init;
        he_now_knows(name);
        return r;
    }

    /* "Set NAME to EXPR." or "Change NAME to EXPR." */
    if (he_accepts(WORD_SET) || he_accepts(WORD_CHANGE)) {
        char name[THE_LIMIT_OF_TEXT];
        the_word *nm = what_he_sees();
        if (nm->what_kind == WORD_STRING_LIT) {
            strncpy(name, nm->the_letters, THE_LIMIT_OF_TEXT - 1);
            he_takes_a_step();
        } else {
            int parts = he_finds_a_known_name(name);
            if (parts == 0)
                the_tower_apologizes(line, "I expected the name of something to set, but found \"%s\".", nm->the_letters);
            for (int p = 0; p < parts; p++) he_takes_a_step();
        }
        he_demands(WORD_TO, "the word \"to\"");
        the_room *value = the_parser_reads_an_expression();
        he_demands(WORD_PERIOD, "a period at the end of the assignment");
        the_room *r = a_new_room(ROOM_ASSIGNMENT, line);
        strncpy(r->the_text, name, THE_LIMIT_OF_TEXT - 1);
        r->the_left = value;
        strncpy(the_one_who_builds.the_last_name, name, THE_LIMIT_OF_TEXT - 1);
        return r;
    }

    /* "Remember EXPR as NAME." — append EXPR to list NAME. */
    if (he_accepts(WORD_REMEMBER)) {
        the_room *value = the_parser_reads_an_expression();
        he_demands(WORD_AS, "the word \"as\" (to say which list to remember it in)");
        char name[THE_LIMIT_OF_TEXT];
        the_word *nm = what_he_sees();
        if (nm->what_kind == WORD_STRING_LIT) { strncpy(name, nm->the_letters, THE_LIMIT_OF_TEXT - 1); he_takes_a_step(); }
        else {
            int parts = he_finds_a_known_name(name);
            if (parts == 0) the_tower_apologizes(line, "After \"as\" I expected the name of a list.");
            for (int p = 0; p < parts; p++) he_takes_a_step();
        }
        he_demands(WORD_PERIOD, "a period at the end");
        the_room *r = a_new_room(ROOM_REMEMBER, line);
        strncpy(r->the_text, name, THE_LIMIT_OF_TEXT - 1);
        r->the_left = value;
        return r;
    }

    /* "Print EXPR [, separated by EXPR]." */
    if (he_accepts(WORD_PRINT)) {
        the_room *value = the_parser_reads_an_expression();
        the_room *sep = NULL;
        if (he_accepts(WORD_COMMA)) {
            he_demands(WORD_SEPARATED, "the word \"separated\"");
            he_demands(WORD_BY,        "the word \"by\"");
            sep = the_parser_reads_an_expression();
        }
        he_demands(WORD_PERIOD, "a period at the end of the print");
        the_room *r = a_new_room(ROOM_PRINT, line);
        r->the_left = value; r->the_separator = sep;
        return r;
    }

    /* "Say EXPR." */
    if (he_accepts(WORD_SAY)) {
        the_room *value = the_parser_reads_an_expression();
        he_demands(WORD_PERIOD, "a period at the end of the say");
        the_room *r = a_new_room(ROOM_SAY, line);
        r->the_left = value;
        return r;
    }

    /* "If COND, STMT." or
     * "If COND, do the following:" + indented block
     * optionally followed by
     * "Otherwise, STMT." or
     * "Otherwise, do the following:" + indented block */
    if (he_accepts(WORD_IF)) {
        the_room *cond = the_parser_reads_a_condition();
        he_demands(WORD_COMMA, "a comma after the condition");
        the_room *then_branch = NULL;
        if (what_he_sees()->what_kind == WORD_DO) {
            he_takes_a_step();
            he_demands(WORD_THE,       "the word \"the\"");
            he_demands(WORD_FOLLOWING, "the word \"following\"");
            he_demands(WORD_COLON,     "a colon");
            then_branch = the_parser_reads_a_block();
        } else {
            then_branch = the_parser_reads_a_statement();
        }
        the_room *else_branch = NULL;
        /* tentatively look past blank lines for an Otherwise */
        int saved = the_one_who_builds.where_he_stands;
        he_skips_blank_lines();
        if (he_accepts(WORD_OTHERWISE)) {
            he_demands(WORD_COMMA, "a comma after \"Otherwise\"");
            if (what_he_sees()->what_kind == WORD_DO) {
                he_takes_a_step();
                he_demands(WORD_THE,       "the word \"the\"");
                he_demands(WORD_FOLLOWING, "the word \"following\"");
                he_demands(WORD_COLON,     "a colon");
                else_branch = the_parser_reads_a_block();
            } else {
                else_branch = the_parser_reads_a_statement();
            }
        } else {
            the_one_who_builds.where_he_stands = saved;
        }
        the_room *r = a_new_room(ROOM_CONDITIONAL, line);
        r->the_left = cond;
        r->the_right = then_branch;
        r->the_destination = else_branch;
        return r;
    }

    /* "For every NAME from EXPR to EXPR, do the following:" + block
     * or
     * "For every NAME in LIST, do the following:" + block */
    if (he_accepts(WORD_FOR)) {
        he_demands(WORD_EVERY, "the word \"every\"");
        the_word *nm = what_he_sees();
        char counter[THE_LIMIT_OF_TEXT];
        if (nm->what_kind == WORD_NAME_WORD) {
            strncpy(counter, nm->the_letters, THE_LIMIT_OF_TEXT - 1);
            he_takes_a_step();
        } else {
            the_tower_apologizes(line, "After \"every\" I expected the name of a counter.");
        }
        he_now_knows(counter);

        the_room *r = a_new_room(ROOM_FOR_LOOP, line);
        strncpy(r->the_counter_name, counter, THE_LIMIT_OF_TEXT - 1);

        if (he_accepts(WORD_IN)) {
            /* The walking-through form: bind the counter to each
             * element of the named list in turn. */
            char listname[THE_LIMIT_OF_TEXT];
            int parts = he_finds_a_known_name(listname);
            if (parts == 0) {
                the_word *bad = what_he_sees();
                the_tower_apologizes(line,
                    "After \"in\" I expected the name of a list, but found \"%s\".",
                    bad->the_letters);
            }
            for (int p = 0; p < parts; p++) he_takes_a_step();
            r->is_foreach = 1;
            strncpy(r->the_text, listname, THE_LIMIT_OF_TEXT - 1);
        } else {
            he_demands(WORD_FROM, "the word \"from\" or \"in\"");
            the_room *start = the_parser_reads_an_expression();
            he_demands(WORD_TO, "the word \"to\"");
            the_room *end = the_parser_reads_an_expression();
            r->the_beginning = start;
            r->the_destination = end;
        }

        he_demands(WORD_COMMA, "a comma after the loop header");
        he_demands(WORD_DO,        "the word \"do\"");
        he_demands(WORD_THE,       "the word \"the\"");
        he_demands(WORD_FOLLOWING, "the word \"following\"");
        he_demands(WORD_COLON,     "a colon");
        the_room *body = the_parser_reads_a_block();
        r->the_left = body;
        strncpy(the_one_who_builds.the_last_name, counter, THE_LIMIT_OF_TEXT - 1);
        return r;
    }

    /* "While COND, do the following:" + block */
    if (he_accepts(WORD_WHILE)) {
        the_room *cond = the_parser_reads_a_condition();
        he_demands(WORD_COMMA, "a comma after the condition");
        he_demands(WORD_DO, "the word \"do\""); he_demands(WORD_THE, "the word \"the\"");
        he_demands(WORD_FOLLOWING, "the word \"following\""); he_demands(WORD_COLON, "a colon");
        the_room *body = the_parser_reads_a_block();
        the_room *r = a_new_room(ROOM_WHILE_LOOP, line);
        r->the_left = cond; r->the_right = body;
        return r;
    }

    /* "To NAME [with NAME and NAME], do the following:" + block */
    if (he_accepts(WORD_TO_DEFINE)) {
        the_word *nm = what_he_sees();
        if (nm->what_kind != WORD_NAME_WORD)
            the_tower_apologizes(line, "After \"To\" I expected the name of a recipe.");
        char rname[THE_LIMIT_OF_TEXT];
        strncpy(rname, nm->the_letters, THE_LIMIT_OF_TEXT - 1);
        he_takes_a_step();
        the_room *r = a_new_room(ROOM_FUNCTION_DEF, line);
        strncpy(r->the_recipe_name, rname, THE_LIMIT_OF_TEXT - 1);

        /* Register the recipe BEFORE reading the body, so the body
         * can call itself by name (the gift of recursion). */
        he_now_knows_a_recipe(rname);

        if (he_accepts(WORD_WITH)) {
            while (1) {
                the_word *p = what_he_sees();
                if (p->what_kind != WORD_NAME_WORD)
                    the_tower_apologizes(line, "After \"with\" I expected an ingredient name.");
                if (r->how_many_ingredients >= THE_LIMIT_OF_PARAMS)
                    the_tower_apologizes(line, "A recipe can have at most %d ingredients.", THE_LIMIT_OF_PARAMS);
                strncpy(r->the_ingredients[r->how_many_ingredients++], p->the_letters, THE_LIMIT_OF_TEXT - 1);
                he_now_knows(p->the_letters);
                he_takes_a_step();
                if (!he_accepts(WORD_AND)) break;
            }
        }
        he_demands(WORD_COMMA, "a comma after the recipe header");
        he_demands(WORD_DO, "the word \"do\""); he_demands(WORD_THE, "the word \"the\"");
        he_demands(WORD_FOLLOWING, "the word \"following\""); he_demands(WORD_COLON, "a colon");
        the_room *body = the_parser_reads_a_block();
        r->the_left = body;
        return r;
    }

    /* "The answer is EXPR." or "Give back EXPR." */
    if (he_accepts(WORD_THE)) {
        he_demands(WORD_ANSWER, "the word \"answer\"");
        he_demands(WORD_IS, "the word \"is\"");
        the_room *value = the_parser_reads_an_expression();
        he_demands(WORD_PERIOD, "a period at the end");
        the_room *r = a_new_room(ROOM_RETURN, line);
        r->the_left = value;
        return r;
    }
    if (he_accepts(WORD_GIVE)) {
        he_demands(WORD_BACK, "the word \"back\"");
        the_room *value = the_parser_reads_an_expression();
        he_demands(WORD_PERIOD, "a period at the end");
        the_room *r = a_new_room(ROOM_RETURN, line);
        r->the_left = value;
        return r;
    }

    /* "Stop." — break out of the innermost loop. */
    if (he_accepts(WORD_STOP)) {
        he_demands(WORD_PERIOD, "a period at the end of \"Stop\"");
        return a_new_room(ROOM_STOP, line);
    }

    /* A bare verb-call: NAME [with EXPR [and EXPR ...]]. */
    if (w->what_kind == WORD_NAME_WORD) {
        char fname[THE_LIMIT_OF_TEXT];
        int parts = he_finds_a_known_name(fname);
        if (parts > 0 && the_name_is_a_recipe(fname)) {
            for (int p = 0; p < parts; p++) he_takes_a_step();
            the_room *r = a_new_room(ROOM_FUNCTION_CALL, line);
            strncpy(r->the_recipe_name, fname, THE_LIMIT_OF_TEXT - 1);
            if (he_accepts(WORD_WITH)) {
                while (1) {
                    if (r->how_many_arguments >= THE_LIMIT_OF_PARAMS)
                        the_tower_apologizes(line, "Too many arguments.");
                    r->the_arguments[r->how_many_arguments++] = the_parser_reads_an_expression();
                    if (!he_accepts(WORD_AND)) break;
                }
            }
            he_demands(WORD_PERIOD, "a period at the end of the call");
            return r;
        }
    }

    the_tower_apologizes(line,
        "I don't know how to begin a sentence with \"%s\".\n  "
        "A sentence in Babel usually starts with one of: Let, Set, Change, Remember,\n  "
        "Print, Say, If, For, While, To, The (answer), Give, or Stop.",
        w->the_letters);
    return NULL;
}

/* The Parser's first and last act: to take the row of words and
 * lift them into a single tower with many floors. */
static the_room* the_parser_builds_the_tower(void) {
    the_one_who_builds.where_he_stands = 0;
    the_one_who_builds.how_many_names = 0;
    the_one_who_builds.the_last_name[0] = '\0';
    the_room *top = a_new_room(ROOM_BLOCK, 1);
    while (what_he_sees()->what_kind != WORD_END_OF_SPEECH) {
        he_skips_blank_lines();
        if (what_he_sees()->what_kind == WORD_END_OF_SPEECH) break;
        the_room *s = the_parser_reads_a_statement();
        if (s) {
            if (top->how_many_rooms_within >= THE_LIMIT_OF_BLOCK)
                the_tower_apologizes(s->which_line,
                    "This manuscript has more sentences at the top level than the "
                    "tower can hold. Consider grouping some into recipes.");
            top->the_rooms_within[top->how_many_rooms_within++] = s;
        }
    }
    return top;
}

/* ================================================================
 * CHAPTER THREE: THE EVALUATOR
 *
 * The Evaluator was neither builder nor architect. She was a
 * prophet. She could not lay bricks or draw plans. But she could
 * walk through the finished tower, room by room, floor by floor,
 * and understand what each room was meant to mean. She could see
 * the intention behind every wall.
 *
 * Where the Lexer found words and the Parser found structure,
 * the Evaluator found truth.
 * ================================================================ */

typedef enum {
    THING_NUMBER,
    THING_STRING,
    THING_BOOLEAN,
    THING_LIST,
    THING_NOTHING,
    THING_FUNCTION
} the_kinds_of_things;

typedef struct the_thing the_thing;
typedef struct the_registry the_registry;

struct the_thing {
    the_kinds_of_things what_it_is;
    double  its_count;
    int     its_truth;
    char    its_utterance[80];

    /* Lists keep their elements on the heap so the_thing itself
     * stays small enough to live in the pool millions at a time. */
    the_thing **its_elements;
    int         how_many_elements;
    int         its_capacity;

    /* Recipes carry a pointer back to the body they were born from
     * and the room they grew up in, so the names that were nearby
     * at their birth remain reachable. */
    the_room      *its_body;
    char           its_params[THE_LIMIT_OF_PARAMS][THE_LIMIT_OF_TEXT];
    int            how_many_params;
    the_registry  *its_birth_scope;
};

struct the_registry {
    char           the_names[THE_LIMIT_OF_NAMES][THE_LIMIT_OF_TEXT];
    the_thing     *what_they_mean[THE_LIMIT_OF_NAMES];
    int            how_many_names;
    the_registry  *the_room_above;   /* parent scope, or NULL */
};

/* The pool of all things ever brought into being. */
static the_thing *the_things_pool;
static int how_many_things_so_far;
static void the_world_is_made(void) {
    the_things_pool = (the_thing*)calloc(THE_LIMIT_OF_THINGS, sizeof(the_thing));
    if (!the_things_pool) {
        fprintf(stderr, "The world could not be made.\n");
        exit(1);
    }
}

static the_thing* a_new_thing(the_kinds_of_things kind) {
    if (how_many_things_so_far >= THE_LIMIT_OF_THINGS)
        the_tower_apologizes(0, "The tower has more things in it than the world has space for.");
    the_thing *t = &the_things_pool[how_many_things_so_far++];
    memset(t, 0, sizeof(*t));
    t->what_it_is = kind;
    return t;
}

static the_thing* a_new_number(double n)   { the_thing *t = a_new_thing(THING_NUMBER); t->its_count = n; return t; }
static the_thing* a_new_string(const char *s) { the_thing *t = a_new_thing(THING_STRING); strncpy(t->its_utterance, s, sizeof(t->its_utterance) - 1); return t; }
static the_thing* a_new_boolean(int b)     { the_thing *t = a_new_thing(THING_BOOLEAN); t->its_truth = b ? 1 : 0; return t; }
static the_thing* a_new_nothing(void)      { return a_new_thing(THING_NOTHING); }
static the_thing* a_new_list(void)         { return a_new_thing(THING_LIST); }

/* Pools for registries — one fresh registry for each function call
 * or loop iteration that needs its own scope. */
#define THE_LIMIT_OF_REGISTRIES 4096
static the_registry the_registries_pool[THE_LIMIT_OF_REGISTRIES];
static int how_many_registries_so_far;

static the_registry* a_fresh_registry(the_registry *parent) {
    if (how_many_registries_so_far >= THE_LIMIT_OF_REGISTRIES)
        the_tower_apologizes(0, "There are too many nested rooms — the tower's recursion is too deep.");
    the_registry *r = &the_registries_pool[how_many_registries_so_far++];
    r->how_many_names = 0;
    r->the_room_above = parent;
    return r;
}

/* The registry's two small acts: writing a name in, and looking
 * a name up. Looking up walks the chain of rooms above. */
static void the_registry_writes(the_registry *here, const char *name, the_thing *meaning) {
    for (int i = 0; i < here->how_many_names; i++) {
        if (strcmp(here->the_names[i], name) == 0) { here->what_they_mean[i] = meaning; return; }
    }
    if (here->how_many_names >= THE_LIMIT_OF_NAMES)
        the_tower_apologizes(0, "Too many names in a single room of the registry.");
    strncpy(here->the_names[here->how_many_names], name, THE_LIMIT_OF_TEXT - 1);
    here->what_they_mean[here->how_many_names++] = meaning;
}

static the_thing* the_registry_recalls(the_registry *here, const char *name) {
    for (the_registry *r = here; r; r = r->the_room_above) {
        for (int i = 0; i < r->how_many_names; i++) {
            if (strcmp(r->the_names[i], name) == 0) return r->what_they_mean[i];
        }
    }
    return NULL;
}

/* Walk up the chain of registries and rebind a name where it
 * already lives, so assignments do not silently shadow. */
static int the_registry_reassigns(the_registry *here, const char *name, the_thing *meaning) {
    for (the_registry *r = here; r; r = r->the_room_above) {
        for (int i = 0; i < r->how_many_names; i++) {
            if (strcmp(r->the_names[i], name) == 0) { r->what_they_mean[i] = meaning; return 1; }
        }
    }
    return 0;
}

/* The act of speaking a thing aloud — turning it into characters
 * the world outside the tower can see. */
static void the_evaluator_speaks_a_thing(the_thing *t, const char *separator) {
    if (!t) { printf("nothing"); return; }
    switch (t->what_it_is) {
        case THING_NUMBER:
            if (t->its_count == (long long)t->its_count) printf("%lld", (long long)t->its_count);
            else printf("%g", t->its_count);
            break;
        case THING_STRING:  printf("%s", t->its_utterance); break;
        case THING_BOOLEAN: printf("%s", t->its_truth ? "true" : "false"); break;
        case THING_NOTHING: printf("nothing"); break;
        case THING_LIST:
            for (int i = 0; i < t->how_many_elements; i++) {
                if (i > 0) printf("%s", separator ? separator : ", ");
                the_evaluator_speaks_a_thing(t->its_elements[i], separator);
            }
            break;
        case THING_FUNCTION: printf("<a recipe>"); break;
    }
}

/* The truth-test that loops and conditionals depend on. Numbers
 * are true when nonzero, strings when nonempty, lists when they
 * contain anything, booleans when they say so. */
static int the_thing_is_true(the_thing *t) {
    if (!t) return 0;
    switch (t->what_it_is) {
        case THING_NUMBER:  return t->its_count != 0;
        case THING_STRING:  return t->its_utterance[0] != '\0';
        case THING_BOOLEAN: return t->its_truth;
        case THING_LIST:    return t->how_many_elements > 0;
        case THING_NOTHING: return 0;
        case THING_FUNCTION:return 1;
    }
    return 0;
}

/* Equality between two things, used by both the comparator and
 * by the wider sense of "is" the language uses. */
static int the_things_are_equal(the_thing *a, the_thing *b) {
    if (!a || !b) return 0;
    if (a->what_it_is != b->what_it_is) {
        if ((a->what_it_is == THING_NUMBER && b->what_it_is == THING_BOOLEAN) ||
            (b->what_it_is == THING_NUMBER && a->what_it_is == THING_BOOLEAN))
            return ((a->what_it_is == THING_NUMBER ? a->its_count : a->its_truth) ==
                    (b->what_it_is == THING_NUMBER ? b->its_count : b->its_truth));
        return 0;
    }
    switch (a->what_it_is) {
        case THING_NUMBER:  return a->its_count == b->its_count;
        case THING_STRING:  return strcmp(a->its_utterance, b->its_utterance) == 0;
        case THING_BOOLEAN: return a->its_truth == b->its_truth;
        case THING_NOTHING: return 1;
        default:            return 0;
    }
}

/* The Evaluator's central act: she enters a room and discovers
 * what it means. The kind of room tells her how to listen. */
static the_thing* the_evaluator_enters(the_room *room, the_registry *where_she_is);

/* A small helper used by binary operations: insist on numbers. */
static double a_number_or_apologies(the_thing *t, int line) {
    if (!t) the_tower_apologizes(line, "I expected a number, but there was nothing here.");
    if (t->what_it_is != THING_NUMBER)
        the_tower_apologizes(line, "I expected a number here, but I found something else.");
    return t->its_count;
}

static the_thing* the_evaluator_enters(the_room *room, the_registry *where_she_is) {
    if (!room) return a_new_nothing();

    switch (room->what_kind) {

    /* The void — nothing in particular. */
    case ROOM_NOTHING: return a_new_nothing();

    /* A bare count. Its meaning is itself. */
    case ROOM_NUMBER:  return a_new_number(room->the_count);

    /* A spoken phrase. Its meaning is the phrase. */
    case ROOM_STRING:  return a_new_string(room->the_text);

    /* A judgement, set in stone. */
    case ROOM_BOOLEAN: return a_new_boolean(room->the_truth);

    /* A name. She walks the registry, room by room above her,
     * until she finds where the name was first written. */
    case ROOM_VARIABLE: {
        the_thing *t = the_registry_recalls(where_she_is, room->the_text);
        if (!t) the_tower_apologizes(room->which_line,
            "I don't know what \"%s\" means.\n  "
            "Perhaps you meant to introduce it earlier with \"Let there be...\"?",
            room->the_text);
        return t;
    }

    /* A measure of how much a list contains. */
    case ROOM_LENGTH: {
        the_thing *t = the_registry_recalls(where_she_is, room->the_text);
        if (!t || t->what_it_is != THING_LIST)
            the_tower_apologizes(room->which_line, "\"%s\" is not a list, so it has no length I can measure.", room->the_text);
        return a_new_number(t->how_many_elements);
    }

    /* The weight of accumulation: the sum of every number in a list. */
    case ROOM_SUM: {
        the_thing *t = the_registry_recalls(where_she_is, room->the_text);
        if (!t || t->what_it_is != THING_LIST)
            the_tower_apologizes(room->which_line, "\"%s\" is not a list, so I can't sum it.", room->the_text);
        double total = 0;
        for (int i = 0; i < t->how_many_elements; i++)
            total += a_number_or_apologies(t->its_elements[i], room->which_line);
        return a_new_number(total);
    }

    /* Two things combined into one according to a small rite
     * — addition, subtraction, comparison, and so on. */
    case ROOM_BINARY_OP: {
        the_thing *L = the_evaluator_enters(room->the_left, where_she_is);
        the_thing *R = the_evaluator_enters(room->the_right, where_she_is);
        const char *op = room->the_text;
        int line = room->which_line;
        if (strcmp(op, "+") == 0) {
            if (L && L->what_it_is == THING_STRING) {
                char buf[THE_LIMIT_OF_TEXT * 2];
                snprintf(buf, sizeof(buf), "%s%s", L->its_utterance,
                         R && R->what_it_is == THING_STRING ? R->its_utterance : "");
                return a_new_string(buf);
            }
            return a_new_number(a_number_or_apologies(L, line) + a_number_or_apologies(R, line));
        }
        if (strcmp(op, "-") == 0) return a_new_number(a_number_or_apologies(L, line) - a_number_or_apologies(R, line));
        if (strcmp(op, "*") == 0) return a_new_number(a_number_or_apologies(L, line) * a_number_or_apologies(R, line));
        if (strcmp(op, "/") == 0) {
            double r = a_number_or_apologies(R, line);
            if (r == 0) the_tower_apologizes(line,
                "You asked me to divide by zero. I can't do that — no one can,\n  "
                "not even in Babel.");
            return a_new_number(a_number_or_apologies(L, line) / r);
        }
        if (strcmp(op, "%") == 0) {
            double r = a_number_or_apologies(R, line);
            if (r == 0) the_tower_apologizes(line, "You asked me to take a remainder modulo zero. There is no such thing.");
            return a_new_number(fmod(a_number_or_apologies(L, line), r));
        }
        if (strcmp(op, ">") == 0)  return a_new_boolean(a_number_or_apologies(L, line) >  a_number_or_apologies(R, line));
        if (strcmp(op, "<") == 0)  return a_new_boolean(a_number_or_apologies(L, line) <  a_number_or_apologies(R, line));
        if (strcmp(op, "==") == 0) return a_new_boolean(the_things_are_equal(L, R));
        if (strcmp(op, "is") == 0) return a_new_boolean(the_things_are_equal(L, R));
        if (strcmp(op, "neq") == 0) return a_new_boolean(!the_things_are_equal(L, R));
        if (strcmp(op, "div") == 0) {
            double r = a_number_or_apologies(R, line);
            if (r == 0) the_tower_apologizes(line, "I can't tell whether something is divisible by zero.");
            return a_new_boolean(fmod(a_number_or_apologies(L, line), r) == 0);
        }
        if (strcmp(op, "ndiv") == 0) {
            double r = a_number_or_apologies(R, line);
            if (r == 0) the_tower_apologizes(line, "I can't tell whether something is not divisible by zero.");
            return a_new_boolean(fmod(a_number_or_apologies(L, line), r) != 0);
        }
        if (strcmp(op, "contains") == 0) {
            if (!L || L->what_it_is != THING_LIST)
                the_tower_apologizes(line, "Only lists can contain things.");
            for (int i = 0; i < L->how_many_elements; i++)
                if (the_things_are_equal(L->its_elements[i], R)) return a_new_boolean(1);
            return a_new_boolean(0);
        }
        the_tower_apologizes(line, "I don't know how to combine two things with \"%s\".", op);
    }

    /* A new name is born here. The Evaluator writes it into the
     * registry of the current room. */
    case ROOM_DECLARATION: {
        the_thing *initial = NULL;
        if (room->the_left && room->the_left->what_kind == ROOM_NOTHING && strcmp(room->the_type, "list") == 0) {
            initial = a_new_list();
        } else if (room->the_left) {
            initial = the_evaluator_enters(room->the_left, where_she_is);
        } else {
            if (strcmp(room->the_type, "number") == 0)  initial = a_new_number(0);
            else if (strcmp(room->the_type, "list") == 0) initial = a_new_list();
            else if (strcmp(room->the_type, "truth") == 0)initial = a_new_boolean(0);
            else if (strcmp(room->the_type, "word") == 0) initial = a_new_string("");
            else                                          initial = a_new_nothing();
        }
        the_registry_writes(where_she_is, room->the_text, initial);
        return initial;
    }

    /* A name is rewritten. The Evaluator finds the existing
     * binding (walking up the rooms above) and copies the new
     * value into it in place. In-place mutation lets variables
     * declared above a loop survive the loop's iteration-by-
     * iteration reset of the things-pool. */
    case ROOM_ASSIGNMENT: {
        the_thing *value = the_evaluator_enters(room->the_left, where_she_is);
        the_thing *existing = the_registry_recalls(where_she_is, room->the_text);
        if (existing) {
            existing->what_it_is = value->what_it_is;
            existing->its_count = value->its_count;
            existing->its_truth = value->its_truth;
            memcpy(existing->its_utterance, value->its_utterance, sizeof(existing->its_utterance));
            existing->its_elements = value->its_elements;
            existing->how_many_elements = value->how_many_elements;
            existing->its_capacity = value->its_capacity;
            return existing;
        }
        the_registry_writes(where_she_is, room->the_text, value);
        return value;
    }

    /* A new element is added to the end of a list. */
    case ROOM_REMEMBER: {
        the_thing *list = the_registry_recalls(where_she_is, room->the_text);
        if (!list || list->what_it_is != THING_LIST)
            the_tower_apologizes(room->which_line, "I can only remember things in a list, but \"%s\" isn't one.", room->the_text);
        the_thing *value = the_evaluator_enters(room->the_left, where_she_is);
        /* List elements are copied to the heap so that they will
         * outlive whatever loop iteration produced them — the
         * iteration-level pool reset would otherwise sweep them
         * away the moment the next iteration began. */
        the_thing *permanent = (the_thing*)malloc(sizeof(the_thing));
        if (!permanent)
            the_tower_apologizes(room->which_line,
                "The tower has run out of room to remember new things.");
        *permanent = *value;
        if (list->how_many_elements >= list->its_capacity) {
            int new_capacity = list->its_capacity ? list->its_capacity * 2 : 16;
            the_thing **grown = (the_thing**)realloc(list->its_elements,
                (size_t)new_capacity * sizeof(the_thing*));
            if (!grown) {
                free(permanent);
                the_tower_apologizes(room->which_line,
                    "The tower could not grow \"%s\" any longer — "
                    "the shelves have run out of space.", room->the_text);
            }
            list->its_elements = grown;
            list->its_capacity = new_capacity;
        }
        list->its_elements[list->how_many_elements++] = permanent;
        return list;
    }

    /* The desire to be heard. */
    case ROOM_PRINT: {
        the_thing *what = the_evaluator_enters(room->the_left, where_she_is);
        const char *sep = ", ";
        char sep_buf[THE_LIMIT_OF_TEXT];
        if (room->the_separator) {
            the_thing *s = the_evaluator_enters(room->the_separator, where_she_is);
            if (s && s->what_it_is == THING_STRING) {
                strncpy(sep_buf, s->its_utterance, THE_LIMIT_OF_TEXT - 1);
                sep_buf[THE_LIMIT_OF_TEXT - 1] = '\0';
                sep = sep_buf;
            }
        }
        the_evaluator_speaks_a_thing(what, sep);
        printf("\n");
        return a_new_nothing();
    }

    /* The simpler desire — say a single thing. */
    case ROOM_SAY: {
        the_thing *what = the_evaluator_enters(room->the_left, where_she_is);
        the_evaluator_speaks_a_thing(what, ", ");
        printf("\n");
        return a_new_nothing();
    }

    /* The fork in the road. The Evaluator listens to the question,
     * then walks down whichever path the answer points to. */
    case ROOM_CONDITIONAL: {
        the_thing *cond = the_evaluator_enters(room->the_left, where_she_is);
        if (the_thing_is_true(cond))
            return the_evaluator_enters(room->the_right, where_she_is);
        if (room->the_destination)
            return the_evaluator_enters(room->the_destination, where_she_is);
        return a_new_nothing();
    }

    /* The promise of repetition: a number that walks from a
     * starting place to a destination, doing the same work at
     * every step it takes. */
    case ROOM_FOR_LOOP: {
        /* The walking-through form: bind the counter to each element
         * of the list in turn, copying its fields into a stable slot
         * so the child registry's name keeps pointing at the right
         * thing even across the per-iteration mercy of forgetting. */
        if (room->is_foreach) {
            the_thing *list = the_registry_recalls(where_she_is, room->the_text);
            if (!list || list->what_it_is != THING_LIST)
                the_tower_apologizes(room->which_line,
                    "I wanted a list to walk through, but \"%s\" is not a list.",
                    room->the_text);
            the_registry *child = a_fresh_registry(where_she_is);
            the_thing *counter = a_new_number(0);
            the_registry_writes(child, room->the_counter_name, counter);
            int things_mark = how_many_things_so_far;
            int registries_mark = how_many_registries_so_far;
            int names_mark = child->how_many_names;
            int total = list->how_many_elements;
            for (int i = 0; i < total; i++) {
                *counter = *list->its_elements[i];
                the_room *body = room->the_left;
                int stopped = 0;
                for (int s = 0; s < body->how_many_rooms_within; s++) {
                    the_room *st = body->the_rooms_within[s];
                    if (st->what_kind == ROOM_STOP) { stopped = 1; break; }
                    the_evaluator_enters(st, child);
                }
                how_many_things_so_far = things_mark;
                how_many_registries_so_far = registries_mark;
                child->how_many_names = names_mark;
                if (stopped) break;
            }
            return a_new_nothing();
        }
        the_thing *start = the_evaluator_enters(room->the_beginning, where_she_is);
        the_thing *end   = the_evaluator_enters(room->the_destination, where_she_is);
        long lo = (long)a_number_or_apologies(start, room->which_line);
        long hi = (long)a_number_or_apologies(end,   room->which_line);
        the_registry *child = a_fresh_registry(where_she_is);
        the_thing *counter = a_new_number((double)lo);
        the_registry_writes(child, room->the_counter_name, counter);
        int things_mark = how_many_things_so_far;
        int registries_mark = how_many_registries_so_far;
        int names_mark = child->how_many_names;
        for (long i = lo; i <= hi; i++) {
            counter->its_count = (double)i;
            the_room *body = room->the_left;
            int stopped = 0;
            for (int s = 0; s < body->how_many_rooms_within; s++) {
                the_room *st = body->the_rooms_within[s];
                if (st->what_kind == ROOM_STOP) { stopped = 1; break; }
                the_evaluator_enters(st, child);
            }
            /* The mercy of forgetting: at the end of each iteration
             * the Evaluator quietly clears every transient room she
             * opened during it. Variables that lived above the loop
             * (and were updated by in-place mutation) are untouched. */
            how_many_things_so_far = things_mark;
            how_many_registries_so_far = registries_mark;
            child->how_many_names = names_mark;
            if (stopped) break;
        }
        return a_new_nothing();
    }

    /* The persistence of condition: while the question still
     * answers yes, the work goes on. */
    case ROOM_WHILE_LOOP: {
        the_registry *child = a_fresh_registry(where_she_is);
        int things_mark = how_many_things_so_far;
        int registries_mark = how_many_registries_so_far;
        int names_mark = child->how_many_names;
        int safety = 0;
        while (1) {
            the_thing *cond = the_evaluator_enters(room->the_left, child);
            int yes = the_thing_is_true(cond);
            if (!yes) break;
            the_room *body = room->the_right;
            int stopped = 0;
            for (int s = 0; s < body->how_many_rooms_within; s++) {
                the_room *st = body->the_rooms_within[s];
                if (st->what_kind == ROOM_STOP) { stopped = 1; break; }
                the_evaluator_enters(st, child);
            }
            how_many_things_so_far = things_mark;
            how_many_registries_so_far = registries_mark;
            child->how_many_names = names_mark;
            if (stopped) break;
            if (++safety > 100000000)
                the_tower_apologizes(room->which_line, "This while-loop has run a hundred million times. I think it may never end.");
        }
        return a_new_nothing();
    }

    /* A recipe is named and stored. The recipe carries with it
     * the room it was born in, so that names defined nearby
     * remain reachable from inside the recipe. */
    case ROOM_FUNCTION_DEF: {
        the_thing *fn = a_new_thing(THING_FUNCTION);
        fn->its_body = room->the_left;
        fn->how_many_params = room->how_many_ingredients;
        for (int i = 0; i < fn->how_many_params; i++) {
            strncpy(fn->its_params[i], room->the_ingredients[i], sizeof(fn->its_params[i]) - 1);
            fn->its_params[i][sizeof(fn->its_params[i]) - 1] = '\0';
        }
        fn->its_birth_scope = where_she_is;
        the_registry_writes(where_she_is, room->the_recipe_name, fn);
        return a_new_nothing();
    }

    /* A recipe is followed. A fresh room is opened in the registry,
     * the ingredients are bound to their names, and the body of the
     * recipe is walked through. If the recipe gives an answer back,
     * that answer is what the call evaluates to. */
    case ROOM_FUNCTION_CALL: {
        the_thing *fn = the_registry_recalls(where_she_is, room->the_recipe_name);
        if (!fn || fn->what_it_is != THING_FUNCTION)
            the_tower_apologizes(room->which_line, "I don't know a recipe called \"%s\".", room->the_recipe_name);
        if (fn->how_many_params != room->how_many_arguments)
            the_tower_apologizes(room->which_line,
                "The recipe \"%s\" expects %d ingredient(s), but you offered %d.",
                room->the_recipe_name, fn->how_many_params, room->how_many_arguments);
        /* Evaluate arguments in the caller's scope first, then open
         * a fresh room for the recipe to live in. The high-water mark
         * is recorded so the room can be reclaimed when the recipe
         * is done — otherwise even simple recursion would exhaust
         * the registry pool in a few steps. */
        /* Evaluate arguments in the caller's scope first. We must
         * copy each argument value into a struct on the C stack
         * so that the things-pool reset (below) does not erase
         * them before the recipe even sees them. */
        the_thing evaluated_copies[THE_LIMIT_OF_PARAMS];
        for (int i = 0; i < fn->how_many_params; i++) {
            the_thing *t = the_evaluator_enters(room->the_arguments[i], where_she_is);
            evaluated_copies[i] = *t;
        }
        int things_before = how_many_things_so_far;
        int registries_before = how_many_registries_so_far;
        the_registry *child = a_fresh_registry(fn->its_birth_scope);
        for (int i = 0; i < fn->how_many_params; i++) {
            the_thing *fresh = a_new_thing(evaluated_copies[i].what_it_is);
            *fresh = evaluated_copies[i];
            the_registry_writes(child, fn->its_params[i], fresh);
        }
        jmp_buf saved;
        memcpy(&saved, &the_homeward_road, sizeof(jmp_buf));
        the_answer_carried_home = NULL;
        if (setjmp(the_homeward_road) == 0) {
            the_evaluator_enters(fn->its_body, child);
        }
        memcpy(&the_homeward_road, &saved, sizeof(jmp_buf));
        /* Snapshot whatever answer the recipe returned, then
         * reclaim everything the recipe allocated, then re-create
         * the answer in the caller's generation. This keeps the
         * things-pool from filling up under deep recursion. */
        the_thing snapshot;
        if (the_answer_carried_home) snapshot = *the_answer_carried_home;
        else                          { snapshot.what_it_is = THING_NOTHING; }
        how_many_registries_so_far = registries_before;
        how_many_things_so_far = things_before;
        the_thing *answer = a_new_thing(snapshot.what_it_is);
        *answer = snapshot;
        return answer;
    }

    /* The answer carried home — the rest of the recipe is left
     * behind, and the climb back to the call begins. */
    case ROOM_RETURN: {
        the_answer_carried_home = the_evaluator_enters(room->the_left, where_she_is);
        longjmp(the_homeward_road, 1);
    }

    /* The mercy of an early ending — handled by the loops above. */
    case ROOM_STOP: return a_new_nothing();

    /* A floor of many rooms, walked one by one. */
    case ROOM_BLOCK: {
        the_thing *last = a_new_nothing();
        for (int i = 0; i < room->how_many_rooms_within; i++) {
            last = the_evaluator_enters(room->the_rooms_within[i], where_she_is);
        }
        return last;
    }

    default:
        the_tower_apologizes(room->which_line, "I don't know how to walk through this kind of room.");
    }
    return a_new_nothing();
}

/* ================================================================
 * CHAPTER FOUR: THE SCRIBE
 *
 * The Scribe came late to the project. The Lexer had her words,
 * the Parser had his rooms, the Evaluator could walk the tower
 * room by room and find the truth in each one — but they were
 * slow. The Evaluator's walks took a long time, because she
 * stopped at every door to remember which kind of room it was.
 *
 * So the Scribe sat down at the foot of the tower and copied it,
 * room by room, into a second language. Not English, but the
 * older, sharper one the machines themselves had been born
 * speaking: C. What she wrote, the C compiler could swallow
 * whole, and from its mouth came a binary as fast as any in
 * the world.
 *
 * The Scribe tried not to change any meaning. For the rooms she
 * fully understood — numbers, loops, lists, conditions — every
 * one came out the other side meaning exactly what it had meant
 * before, only faster. For the rooms whose meaning depended on
 * things the older, sharper language did not speak as naturally
 * — words compared against other words, phrases stitched together
 * out of parts — she did her best, and where she could not be
 * sure, she wrote a smaller, safer version and let the Evaluator
 * keep the harder readings for herself.
 *
 * This was the honest thing. A Scribe who promises to copy every
 * meaning perfectly is a Scribe who is lying about the size of
 * the world.
 * ================================================================ */

typedef enum { CTY_DOUBLE, CTY_LIST, CTY_BOOL, CTY_STRING, CTY_VOID } c_type;
typedef struct { char name[THE_LIMIT_OF_TEXT]; c_type type; } the_scribes_entry;
static the_scribes_entry the_scribes_dictionary[THE_LIMIT_OF_NAMES * 4];
static int how_many_in_dictionary;

static c_type the_scribe_recalls(const char *name) {
    for (int i = how_many_in_dictionary - 1; i >= 0; i--)
        if (strcmp(the_scribes_dictionary[i].name, name) == 0)
            return the_scribes_dictionary[i].type;
    return CTY_DOUBLE;
}
static void the_scribe_writes_down(const char *name, c_type t) {
    for (int i = 0; i < how_many_in_dictionary; i++) {
        if (strcmp(the_scribes_dictionary[i].name, name) == 0) {
            the_scribes_dictionary[i].type = t; return;
        }
    }
    if (how_many_in_dictionary >= (int)(sizeof(the_scribes_dictionary)/sizeof(the_scribes_entry))) return;
    strncpy(the_scribes_dictionary[how_many_in_dictionary].name, name, THE_LIMIT_OF_TEXT - 1);
    the_scribes_dictionary[how_many_in_dictionary].type = t;
    how_many_in_dictionary++;
}

/* The Scribe's small habit: she takes any English name, and
 * turns it into a C identifier by replacing every space with
 * a quiet underscore, and prefixing it with the letter that
 * tells her whether it is a value or a recipe. */
static void the_scribe_renders_a_name(FILE *out, const char *prefix, const char *name) {
    fprintf(out, "%s", prefix);
    for (const char *c = name; *c; c++) {
        if (isalnum((unsigned char)*c)) fputc(*c, out);
        else fputc('_', out);
    }
}

static void the_scribe_renders_an_expression(FILE *out, the_room *r);

static void the_scribe_renders_a_string_literal(FILE *out, const char *s) {
    fputc('"', out);
    for (const char *c = s; *c; c++) {
        if (*c == '"' || *c == '\\') { fputc('\\', out); fputc(*c, out); }
        else if (*c == '\n') fputs("\\n", out);
        else fputc(*c, out);
    }
    fputc('"', out);
}

static void the_scribe_renders_an_expression(FILE *out, the_room *r) {
    if (!r) { fputs("0", out); return; }
    switch (r->what_kind) {
    case ROOM_NUMBER:  fprintf(out, "((double)%.17g)", r->the_count); return;
    case ROOM_BOOLEAN: fprintf(out, "%d", r->the_truth); return;
    case ROOM_STRING:  the_scribe_renders_a_string_literal(out, r->the_text); return;
    case ROOM_NOTHING: fputs("0", out); return;
    case ROOM_VARIABLE:
        the_scribe_renders_a_name(out, "v_", r->the_text);
        return;
    case ROOM_LENGTH:
        fputs("((double)", out);
        the_scribe_renders_a_name(out, "v_", r->the_text);
        fputs(".n)", out);
        return;
    case ROOM_SUM:
        fputs("babel_sum(&", out);
        the_scribe_renders_a_name(out, "v_", r->the_text);
        fputs(")", out);
        return;
    case ROOM_BINARY_OP: {
        const char *op = r->the_text;
        if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || strcmp(op, "*") == 0 || strcmp(op, "/") == 0) {
            fputs("(", out); the_scribe_renders_an_expression(out, r->the_left);
            fprintf(out, " %s ", op);
            the_scribe_renders_an_expression(out, r->the_right); fputs(")", out);
        } else if (strcmp(op, "%") == 0) {
            fputs("fmod(", out); the_scribe_renders_an_expression(out, r->the_left);
            fputs(", ", out); the_scribe_renders_an_expression(out, r->the_right); fputs(")", out);
        } else if (strcmp(op, "<") == 0 || strcmp(op, ">") == 0 || strcmp(op, "==") == 0 || strcmp(op, "is") == 0) {
            const char *cop = (strcmp(op, "is") == 0) ? "==" : op;
            fputs("(", out); the_scribe_renders_an_expression(out, r->the_left);
            fprintf(out, " %s ", cop);
            the_scribe_renders_an_expression(out, r->the_right); fputs(")", out);
        } else if (strcmp(op, "neq") == 0) {
            fputs("(", out); the_scribe_renders_an_expression(out, r->the_left);
            fputs(" != ", out);
            the_scribe_renders_an_expression(out, r->the_right); fputs(")", out);
        } else if (strcmp(op, "div") == 0) {
            fputs("(fmod(", out); the_scribe_renders_an_expression(out, r->the_left);
            fputs(", ", out); the_scribe_renders_an_expression(out, r->the_right); fputs(") == 0)", out);
        } else if (strcmp(op, "ndiv") == 0) {
            fputs("(fmod(", out); the_scribe_renders_an_expression(out, r->the_left);
            fputs(", ", out); the_scribe_renders_an_expression(out, r->the_right); fputs(") != 0)", out);
        } else if (strcmp(op, "contains") == 0) {
            /* In this younger, sharper tongue the Scribe can only
             * ask the question of a list of numbers. */
            if (!r->the_left || r->the_left->what_kind != ROOM_VARIABLE)
                the_tower_apologizes(r->which_line,
                    "The Scribe can only ask \"contains\" of a named list.");
            fputs("babel_contains(&", out);
            the_scribe_renders_a_name(out, "v_", r->the_left->the_text);
            fputs(", ", out);
            the_scribe_renders_an_expression(out, r->the_right);
            fputs(")", out);
        } else {
            the_tower_apologizes(r->which_line, "The Scribe doesn't yet know how to write \"%s\" in C.", op);
        }
        return;
    }
    case ROOM_FUNCTION_CALL: {
        the_scribe_renders_a_name(out, "f_", r->the_recipe_name);
        fputc('(', out);
        for (int i = 0; i < r->how_many_arguments; i++) {
            if (i > 0) fputs(", ", out);
            the_scribe_renders_an_expression(out, r->the_arguments[i]);
        }
        fputc(')', out);
        return;
    }
    default:
        the_tower_apologizes(r->which_line, "The Scribe doesn't yet know how to render this kind of room as an expression.");
    }
}

static void the_scribe_renders_a_statement(FILE *out, the_room *r, int depth);

static void the_scribe_indents(FILE *out, int depth) {
    for (int i = 0; i < depth; i++) fputs("    ", out);
}

static void the_scribe_renders_a_block(FILE *out, the_room *block, int depth) {
    if (!block) return;
    if (block->what_kind != ROOM_BLOCK) {
        the_scribe_renders_a_statement(out, block, depth);
        return;
    }
    for (int i = 0; i < block->how_many_rooms_within; i++) {
        the_scribe_renders_a_statement(out, block->the_rooms_within[i], depth);
    }
}

static void the_scribe_renders_a_statement(FILE *out, the_room *r, int depth) {
    if (!r) return;
    switch (r->what_kind) {
    case ROOM_DECLARATION: {
        c_type t = CTY_DOUBLE;
        if (strcmp(r->the_type, "list") == 0)  t = CTY_LIST;
        else if (strcmp(r->the_type, "truth") == 0) t = CTY_BOOL;
        else if (strcmp(r->the_type, "word") == 0)  t = CTY_STRING;
        the_scribe_writes_down(r->the_text, t);
        the_scribe_indents(out, depth);
        if (t == CTY_LIST) {
            fputs("babel_list ", out);
            the_scribe_renders_a_name(out, "v_", r->the_text);
            fputs(" = {0};\n", out);
        } else if (t == CTY_BOOL) {
            fputs("int ", out);
            the_scribe_renders_a_name(out, "v_", r->the_text);
            fputs(" = ", out);
            if (r->the_left) the_scribe_renders_an_expression(out, r->the_left);
            else fputs("0", out);
            fputs(";\n", out);
        } else if (t == CTY_STRING) {
            fputs("const char *", out);
            the_scribe_renders_a_name(out, "v_", r->the_text);
            fputs(" = ", out);
            if (r->the_left && r->the_left->what_kind == ROOM_STRING)
                the_scribe_renders_a_string_literal(out, r->the_left->the_text);
            else fputs("\"\"", out);
            fputs(";\n", out);
        } else {
            fputs("double ", out);
            the_scribe_renders_a_name(out, "v_", r->the_text);
            fputs(" = ", out);
            if (r->the_left) the_scribe_renders_an_expression(out, r->the_left);
            else fputs("0", out);
            fputs(";\n", out);
        }
        return;
    }
    case ROOM_ASSIGNMENT: {
        the_scribe_indents(out, depth);
        the_scribe_renders_a_name(out, "v_", r->the_text);
        fputs(" = ", out);
        the_scribe_renders_an_expression(out, r->the_left);
        fputs(";\n", out);
        return;
    }
    case ROOM_REMEMBER: {
        the_scribe_indents(out, depth);
        fputs("babel_push(&", out);
        the_scribe_renders_a_name(out, "v_", r->the_text);
        fputs(", ", out);
        the_scribe_renders_an_expression(out, r->the_left);
        fputs(");\n", out);
        return;
    }
    case ROOM_PRINT: {
        the_scribe_indents(out, depth);
        the_room *e = r->the_left;
        c_type et = CTY_DOUBLE;
        if (e && e->what_kind == ROOM_VARIABLE) et = the_scribe_recalls(e->the_text);
        else if (e && e->what_kind == ROOM_STRING) et = CTY_STRING;
        else if (e && e->what_kind == ROOM_BOOLEAN) et = CTY_BOOL;
        if (e && e->what_kind == ROOM_VARIABLE && et == CTY_LIST) {
            fputs("babel_print_list(&", out);
            the_scribe_renders_a_name(out, "v_", e->the_text);
            fputs(", ", out);
            if (r->the_separator) the_scribe_renders_an_expression(out, r->the_separator);
            else fputs("\", \"", out);
            fputs(");\n", out);
        } else if (et == CTY_STRING) {
            fputs("babel_print_word(", out);
            the_scribe_renders_an_expression(out, e);
            fputs(");\n", out);
        } else if (et == CTY_BOOL) {
            fputs("babel_print_truth(", out);
            the_scribe_renders_an_expression(out, e);
            fputs(");\n", out);
        } else {
            fputs("babel_print_number(", out);
            the_scribe_renders_an_expression(out, e);
            fputs(");\n", out);
        }
        return;
    }
    case ROOM_SAY: {
        the_scribe_indents(out, depth);
        the_room *e = r->the_left;
        if (e && e->what_kind == ROOM_STRING) {
            fputs("printf(\"%s\\n\", ", out);
            the_scribe_renders_a_string_literal(out, e->the_text);
            fputs(");\n", out);
        } else {
            fputs("babel_print_number(", out);
            the_scribe_renders_an_expression(out, e);
            fputs(");\n", out);
        }
        return;
    }
    case ROOM_CONDITIONAL: {
        the_scribe_indents(out, depth);
        fputs("if (", out);
        the_scribe_renders_an_expression(out, r->the_left);
        fputs(") {\n", out);
        the_scribe_renders_a_statement(out, r->the_right, depth + 1);
        the_scribe_indents(out, depth); fputs("}", out);
        if (r->the_destination) {
            fputs(" else {\n", out);
            the_scribe_renders_a_statement(out, r->the_destination, depth + 1);
            the_scribe_indents(out, depth); fputs("}\n", out);
        } else {
            fputs("\n", out);
        }
        return;
    }
    case ROOM_FOR_LOOP: {
        the_scribe_writes_down(r->the_counter_name, CTY_DOUBLE);
        if (r->is_foreach) {
            /* Walk the elements of a named list. The C backend only
             * knows lists of numbers, so the counter is a double. */
            the_scribe_indents(out, depth);
            fputs("for (int ", out);
            the_scribe_renders_a_name(out, "i_", r->the_counter_name);
            fputs(" = 0; ", out);
            the_scribe_renders_a_name(out, "i_", r->the_counter_name);
            fputs(" < ", out);
            the_scribe_renders_a_name(out, "v_", r->the_text);
            fputs(".n; ", out);
            the_scribe_renders_a_name(out, "i_", r->the_counter_name);
            fputs("++) {\n", out);
            the_scribe_indents(out, depth + 1);
            fputs("double ", out);
            the_scribe_renders_a_name(out, "v_", r->the_counter_name);
            fputs(" = ", out);
            the_scribe_renders_a_name(out, "v_", r->the_text);
            fputs(".data[", out);
            the_scribe_renders_a_name(out, "i_", r->the_counter_name);
            fputs("];\n", out);
            the_scribe_renders_a_block(out, r->the_left, depth + 1);
            the_scribe_indents(out, depth); fputs("}\n", out);
            return;
        }
        the_scribe_indents(out, depth);
        fputs("for (long ", out);
        the_scribe_renders_a_name(out, "i_", r->the_counter_name);
        fputs(" = (long)(", out);
        the_scribe_renders_an_expression(out, r->the_beginning);
        fputs("); ", out);
        the_scribe_renders_a_name(out, "i_", r->the_counter_name);
        fputs(" <= (long)(", out);
        the_scribe_renders_an_expression(out, r->the_destination);
        fputs("); ", out);
        the_scribe_renders_a_name(out, "i_", r->the_counter_name);
        fputs("++) {\n", out);
        the_scribe_indents(out, depth + 1);
        fputs("double ", out);
        the_scribe_renders_a_name(out, "v_", r->the_counter_name);
        fputs(" = (double)", out);
        the_scribe_renders_a_name(out, "i_", r->the_counter_name);
        fputs(";\n", out);
        the_scribe_renders_a_block(out, r->the_left, depth + 1);
        the_scribe_indents(out, depth); fputs("}\n", out);
        return;
    }
    case ROOM_WHILE_LOOP: {
        the_scribe_indents(out, depth);
        fputs("while (", out);
        the_scribe_renders_an_expression(out, r->the_left);
        fputs(") {\n", out);
        the_scribe_renders_a_block(out, r->the_right, depth + 1);
        the_scribe_indents(out, depth); fputs("}\n", out);
        return;
    }
    case ROOM_RETURN: {
        the_scribe_indents(out, depth);
        fputs("return ", out);
        the_scribe_renders_an_expression(out, r->the_left);
        fputs(";\n", out);
        return;
    }
    case ROOM_STOP: {
        the_scribe_indents(out, depth);
        fputs("break;\n", out);
        return;
    }
    case ROOM_FUNCTION_CALL: {
        the_scribe_indents(out, depth);
        fputs("(void)", out);
        the_scribe_renders_an_expression(out, r);
        fputs(";\n", out);
        return;
    }
    case ROOM_BLOCK: {
        the_scribe_renders_a_block(out, r, depth);
        return;
    }
    case ROOM_FUNCTION_DEF:
        /* Function definitions are emitted in a pre-pass; here
         * the Scribe steps quietly past them. */
        return;
    default:
        the_tower_apologizes(r->which_line, "The Scribe doesn't yet know how to render this kind of room.");
    }
}

/* The pre-pass: walk the top-level tower, find every recipe, and
 * write it out as a C function before main(). */
static void the_scribe_renders_the_recipes(FILE *out, the_room *top) {
    if (!top || top->what_kind != ROOM_BLOCK) return;
    for (int i = 0; i < top->how_many_rooms_within; i++) {
        the_room *r = top->the_rooms_within[i];
        if (r->what_kind != ROOM_FUNCTION_DEF) continue;
        the_scribe_writes_down(r->the_recipe_name, CTY_DOUBLE);
        fputs("static double ", out);
        the_scribe_renders_a_name(out, "f_", r->the_recipe_name);
        fputc('(', out);
        for (int p = 0; p < r->how_many_ingredients; p++) {
            if (p > 0) fputs(", ", out);
            fputs("double ", out);
            the_scribe_renders_a_name(out, "v_", r->the_ingredients[p]);
            the_scribe_writes_down(r->the_ingredients[p], CTY_DOUBLE);
        }
        fputs(") {\n", out);
        the_scribe_renders_a_block(out, r->the_left, 1);
        fputs("    return 0;\n}\n\n", out);
    }
}

/* The Scribe's complete act: she writes the entire C program from
 * the top of the tower down to its foundation, and then asks the
 * C compiler standing nearby to turn her writing into a binary. */
static int the_scribe_compiles(the_room *top, const char *output_path) {
    char tmp_c[256];
    snprintf(tmp_c, sizeof(tmp_c), "/tmp/babel_%d.c", (int)getpid());
    FILE *out = fopen(tmp_c, "w");
    if (!out) the_tower_apologizes(0, "I couldn't open a place to write the C source.");

    /* The runtime: a few small helpers the generated code can lean on. */
    fputs(
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "#include <string.h>\n"
        "#include <math.h>\n"
        "\n"
        "typedef struct { double *data; int n; int cap; } babel_list;\n"
        "static void babel_push(babel_list *l, double v) {\n"
        "    if (l->n >= l->cap) {\n"
        "        l->cap = l->cap ? l->cap * 2 : 16;\n"
        "        l->data = (double*)realloc(l->data, (size_t)l->cap * sizeof(double));\n"
        "    }\n"
        "    l->data[l->n++] = v;\n"
        "}\n"
        "static double babel_sum(babel_list *l) {\n"
        "    double s = 0; for (int i = 0; i < l->n; i++) s += l->data[i]; return s;\n"
        "}\n"
        "static void babel_print_number(double x) {\n"
        "    if (x == (long long)x) printf(\"%lld\\n\", (long long)x);\n"
        "    else printf(\"%g\\n\", x);\n"
        "}\n"
        "static void babel_print_truth(int x) {\n"
        "    printf(\"%s\\n\", x ? \"true\" : \"false\");\n"
        "}\n"
        "static void babel_print_word(const char *s) {\n"
        "    printf(\"%s\\n\", s ? s : \"\");\n"
        "}\n"
        "static int babel_contains(babel_list *l, double v) {\n"
        "    for (int i = 0; i < l->n; i++) if (l->data[i] == v) return 1;\n"
        "    return 0;\n"
        "}\n"
        "static int babel_words_equal(const char *a, const char *b) {\n"
        "    if (!a || !b) return a == b;\n"
        "    return strcmp(a, b) == 0;\n"
        "}\n"
        "static void babel_print_list(babel_list *l, const char *sep) {\n"
        "    for (int i = 0; i < l->n; i++) {\n"
        "        if (i) printf(\"%s\", sep);\n"
        "        if (l->data[i] == (long long)l->data[i]) printf(\"%lld\", (long long)l->data[i]);\n"
        "        else printf(\"%g\", l->data[i]);\n"
        "    }\n"
        "    printf(\"\\n\");\n"
        "}\n"
        "\n",
        out);

    how_many_in_dictionary = 0;
    the_scribe_renders_the_recipes(out, top);

    fputs("int main(void) {\n", out);
    if (top && top->what_kind == ROOM_BLOCK) {
        for (int i = 0; i < top->how_many_rooms_within; i++) {
            the_room *r = top->the_rooms_within[i];
            if (r->what_kind == ROOM_FUNCTION_DEF) continue;
            the_scribe_renders_a_statement(out, r, 1);
        }
    }
    fputs("    return 0;\n}\n", out);
    fclose(out);

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "cc -O3 -w -o %s %s -lm", output_path, tmp_c);
    int rc = system(cmd);
    /* leave the C file behind for the curious; the path is printed below */
    fprintf(stderr, "Babel: emitted %s, compiled to %s\n", tmp_c, output_path);
    return rc;
}

/* ================================================================
 * CHAPTER FOUR-AND-A-HALF: THE SCRIBE LEARNS A YOUNGER TONGUE
 *
 * After she finished her translation into C, the Scribe heard of
 * another language — younger than C, gentler, whose speakers
 * valued being understood at a glance more than being fast. They
 * called it Python, and they asked if she would write the tower
 * out for them too.
 *
 * She agreed, on one condition: the Python translation would be
 * a record, not a race. It would run no faster than the Evaluator
 * herself — it would only be there for the readers who preferred
 * to see the tower written in their own language first, and then
 * compare.
 *
 * So she opened a second scroll, and beside each C room she had
 * written, she wrote its Python twin. The two scrolls agreed on
 * every meaning that mattered, and where they differed, the
 * Scribe marked it plainly, so no one would be misled.
 * ================================================================ */

static void the_scribe_renders_python_expression(FILE *out, the_room *r);

static void the_scribe_renders_python_name(FILE *out, const char *name) {
    /* Python accepts identifiers with underscores but not spaces. */
    for (const char *p = name; *p; p++)
        fputc((*p == ' ' || *p == '-') ? '_' : *p, out);
}

static void the_scribe_renders_python_string(FILE *out, const char *s) {
    fputc('"', out);
    for (const char *p = s; *p; p++) {
        if (*p == '"' || *p == '\\') fputc('\\', out);
        fputc(*p, out);
    }
    fputc('"', out);
}

static void the_scribe_renders_python_expression(FILE *out, the_room *r) {
    if (!r) { fputs("None", out); return; }
    switch (r->what_kind) {
    case ROOM_NUMBER:
        /* Emit whole numbers as ints so Python division behaves. */
        if (r->the_count == (long long)r->the_count)
            fprintf(out, "%lld", (long long)r->the_count);
        else
            fprintf(out, "%.17g", r->the_count);
        return;
    case ROOM_BOOLEAN: fputs(r->the_truth ? "True" : "False", out); return;
    case ROOM_STRING:  the_scribe_renders_python_string(out, r->the_text); return;
    case ROOM_NOTHING: fputs("None", out); return;
    case ROOM_VARIABLE:
        the_scribe_renders_python_name(out, r->the_text);
        return;
    case ROOM_LENGTH:
        fputs("len(", out);
        the_scribe_renders_python_name(out, r->the_text);
        fputs(")", out);
        return;
    case ROOM_SUM:
        fputs("sum(", out);
        the_scribe_renders_python_name(out, r->the_text);
        fputs(")", out);
        return;
    case ROOM_BINARY_OP: {
        const char *op = r->the_text;
        const char *py = NULL;
        if (strcmp(op, "+") == 0) py = "+";
        else if (strcmp(op, "-") == 0) py = "-";
        else if (strcmp(op, "*") == 0) py = "*";
        else if (strcmp(op, "/") == 0) py = "/";
        else if (strcmp(op, "%") == 0) py = "%";
        else if (strcmp(op, "<") == 0) py = "<";
        else if (strcmp(op, ">") == 0) py = ">";
        else if (strcmp(op, "==") == 0 || strcmp(op, "is") == 0) py = "==";
        else if (strcmp(op, "neq") == 0) py = "!=";
        if (py) {
            fputs("(", out);
            the_scribe_renders_python_expression(out, r->the_left);
            fprintf(out, " %s ", py);
            the_scribe_renders_python_expression(out, r->the_right);
            fputs(")", out);
            return;
        }
        if (strcmp(op, "div") == 0) {
            fputs("((", out);
            the_scribe_renders_python_expression(out, r->the_left);
            fputs(" % ", out);
            the_scribe_renders_python_expression(out, r->the_right);
            fputs(") == 0)", out);
            return;
        }
        if (strcmp(op, "ndiv") == 0) {
            fputs("((", out);
            the_scribe_renders_python_expression(out, r->the_left);
            fputs(" % ", out);
            the_scribe_renders_python_expression(out, r->the_right);
            fputs(") != 0)", out);
            return;
        }
        if (strcmp(op, "contains") == 0) {
            fputs("(", out);
            the_scribe_renders_python_expression(out, r->the_right);
            fputs(" in ", out);
            the_scribe_renders_python_expression(out, r->the_left);
            fputs(")", out);
            return;
        }
        the_tower_apologizes(r->which_line, "The Scribe doesn't yet know how to write \"%s\" in Python.", op);
        return;
    }
    case ROOM_FUNCTION_CALL: {
        the_scribe_renders_python_name(out, r->the_recipe_name);
        fputc('(', out);
        for (int i = 0; i < r->how_many_arguments; i++) {
            if (i > 0) fputs(", ", out);
            the_scribe_renders_python_expression(out, r->the_arguments[i]);
        }
        fputc(')', out);
        return;
    }
    default:
        the_tower_apologizes(r->which_line, "The Scribe doesn't yet know how to render this kind of room in Python.");
    }
}

static void the_scribe_renders_python_indent(FILE *out, int depth) {
    for (int i = 0; i < depth; i++) fputs("    ", out);
}

static void the_scribe_renders_python_statement(FILE *out, the_room *r, int depth);

static void the_scribe_renders_python_block(FILE *out, the_room *block, int depth) {
    if (!block) {
        the_scribe_renders_python_indent(out, depth);
        fputs("pass\n", out);
        return;
    }
    if (block->what_kind != ROOM_BLOCK) {
        the_scribe_renders_python_statement(out, block, depth);
        return;
    }
    if (block->how_many_rooms_within == 0) {
        the_scribe_renders_python_indent(out, depth);
        fputs("pass\n", out);
        return;
    }
    for (int i = 0; i < block->how_many_rooms_within; i++)
        the_scribe_renders_python_statement(out, block->the_rooms_within[i], depth);
}

static void the_scribe_renders_python_statement(FILE *out, the_room *r, int depth) {
    if (!r) return;
    switch (r->what_kind) {
    case ROOM_DECLARATION: {
        int init_is_nothing = r->the_left && r->the_left->what_kind == ROOM_NOTHING;
        the_scribe_renders_python_indent(out, depth);
        the_scribe_renders_python_name(out, r->the_text);
        fputs(" = ", out);
        if (strcmp(r->the_type, "list") == 0 && (init_is_nothing || !r->the_left)) {
            fputs("[]", out);
        } else if (r->the_left && !init_is_nothing) {
            the_scribe_renders_python_expression(out, r->the_left);
        } else if (strcmp(r->the_type, "truth") == 0) {
            fputs("False", out);
        } else if (strcmp(r->the_type, "word") == 0) {
            fputs("\"\"", out);
        } else {
            fputs("0", out);
        }
        fputs("\n", out);
        return;
    }
    case ROOM_ASSIGNMENT:
        the_scribe_renders_python_indent(out, depth);
        the_scribe_renders_python_name(out, r->the_text);
        fputs(" = ", out);
        the_scribe_renders_python_expression(out, r->the_left);
        fputs("\n", out);
        return;
    case ROOM_REMEMBER:
        the_scribe_renders_python_indent(out, depth);
        the_scribe_renders_python_name(out, r->the_text);
        fputs(".append(", out);
        the_scribe_renders_python_expression(out, r->the_left);
        fputs(")\n", out);
        return;
    case ROOM_PRINT: {
        the_scribe_renders_python_indent(out, depth);
        the_room *e = r->the_left;
        if (r->the_separator) {
            /* print with a separator: join a list, or fall back to str(). */
            fputs("print(", out);
            if (r->the_separator) {
                the_scribe_renders_python_expression(out, r->the_separator);
                fputs(".join(str(__x) for __x in ", out);
                the_scribe_renders_python_expression(out, e);
                fputs("))", out);
            }
            fputs("\n", out);
        } else {
            fputs("print(", out);
            the_scribe_renders_python_expression(out, e);
            fputs(")\n", out);
        }
        return;
    }
    case ROOM_SAY:
        the_scribe_renders_python_indent(out, depth);
        fputs("print(", out);
        the_scribe_renders_python_expression(out, r->the_left);
        fputs(")\n", out);
        return;
    case ROOM_CONDITIONAL:
        the_scribe_renders_python_indent(out, depth);
        fputs("if ", out);
        the_scribe_renders_python_expression(out, r->the_left);
        fputs(":\n", out);
        the_scribe_renders_python_block(out, r->the_right, depth + 1);
        if (r->the_destination) {
            the_scribe_renders_python_indent(out, depth);
            fputs("else:\n", out);
            the_scribe_renders_python_block(out, r->the_destination, depth + 1);
        }
        return;
    case ROOM_FOR_LOOP:
        the_scribe_renders_python_indent(out, depth);
        if (r->is_foreach) {
            fputs("for ", out);
            the_scribe_renders_python_name(out, r->the_counter_name);
            fputs(" in ", out);
            the_scribe_renders_python_name(out, r->the_text);
            fputs(":\n", out);
            the_scribe_renders_python_block(out, r->the_left, depth + 1);
            return;
        }
        fputs("for ", out);
        the_scribe_renders_python_name(out, r->the_counter_name);
        fputs(" in range(int(", out);
        the_scribe_renders_python_expression(out, r->the_beginning);
        fputs("), int(", out);
        the_scribe_renders_python_expression(out, r->the_destination);
        fputs(") + 1):\n", out);
        the_scribe_renders_python_block(out, r->the_left, depth + 1);
        return;
    case ROOM_WHILE_LOOP:
        the_scribe_renders_python_indent(out, depth);
        fputs("while ", out);
        the_scribe_renders_python_expression(out, r->the_left);
        fputs(":\n", out);
        the_scribe_renders_python_block(out, r->the_right, depth + 1);
        return;
    case ROOM_RETURN:
        the_scribe_renders_python_indent(out, depth);
        fputs("return ", out);
        the_scribe_renders_python_expression(out, r->the_left);
        fputs("\n", out);
        return;
    case ROOM_STOP:
        the_scribe_renders_python_indent(out, depth);
        fputs("break\n", out);
        return;
    case ROOM_FUNCTION_CALL:
        the_scribe_renders_python_indent(out, depth);
        the_scribe_renders_python_expression(out, r);
        fputs("\n", out);
        return;
    case ROOM_BLOCK:
        the_scribe_renders_python_block(out, r, depth);
        return;
    case ROOM_FUNCTION_DEF:
        /* Function definitions are emitted in a pre-pass. */
        return;
    default:
        the_tower_apologizes(r->which_line, "The Scribe doesn't know how to translate this room into Python.");
    }
}

static void the_scribe_renders_python_recipes(FILE *out, the_room *top) {
    if (!top || top->what_kind != ROOM_BLOCK) return;
    for (int i = 0; i < top->how_many_rooms_within; i++) {
        the_room *r = top->the_rooms_within[i];
        if (r->what_kind != ROOM_FUNCTION_DEF) continue;
        fputs("def ", out);
        the_scribe_renders_python_name(out, r->the_recipe_name);
        fputc('(', out);
        for (int p = 0; p < r->how_many_ingredients; p++) {
            if (p > 0) fputs(", ", out);
            the_scribe_renders_python_name(out, r->the_ingredients[p]);
        }
        fputs("):\n", out);
        the_scribe_renders_python_block(out, r->the_left, 1);
        fputs("\n", out);
    }
}

/* The complete Python transcription: the Scribe walks the tower
 * once, writing a Python twin for every room that has one. */
static int the_scribe_transcribes_into_python(the_room *top, const char *output_path) {
    FILE *out = output_path ? fopen(output_path, "w") : stdout;
    if (!out) the_tower_apologizes(0, "I couldn't open a place to write the Python source.");

    fputs("# This Python file was transcribed from a Babel manuscript.\n", out);
    fputs("# The Scribe wrote it so readers of Python could see the tower\n", out);
    fputs("# in their own language. Run it with `python3 <this-file>`.\n\n", out);

    the_scribe_renders_python_recipes(out, top);

    if (top && top->what_kind == ROOM_BLOCK) {
        for (int i = 0; i < top->how_many_rooms_within; i++) {
            the_room *r = top->the_rooms_within[i];
            if (r->what_kind == ROOM_FUNCTION_DEF) continue;
            the_scribe_renders_python_statement(out, r, 0);
        }
    }

    if (output_path) {
        fclose(out);
        fprintf(stderr, "Babel: wrote a Python transcription to %s\n", output_path);
    }
    return 0;
}

/* ================================================================
 * CHAPTER EIGHT: THE INTERPRETER OF TONGUES
 *
 * In the order of the story, this chapter comes last. In the
 * order of the tower, it comes first. Before the Lexer, before
 * the Parser, before any builder laid a brick, there was the
 * gate. And at the gate stood the Interpreter of Tongues.
 *
 * She was not a builder. She could not lay bricks or draw plans
 * or walk the finished floors finding truth. Her gift was older
 * and simpler: she could listen to a stranger speak in their own
 * tongue and find the Babel words hidden inside their meaning.
 *
 * She never assumed she understood. She always asked. That was
 * her only rule, and it was the rule that kept the tower
 * standing. A builder who does not ask will raise the wrong
 * walls; an interpreter who does not ask will translate the
 * wrong words; and the tower will fall either way.
 *
 * She carried a book. In it were the words travelers commonly
 * used and the Babel words they most likely intended. The book
 * was not complete — no book of translations ever is — but it
 * covered the most common misunderstandings. She updated it
 * whenever a new traveler taught her a new way of saying an old
 * thing.
 * ================================================================ */

/* A single page in the Interpreter's book: what travelers tend
 * to say, what Babel would say in its place, and a short note
 * on why the translation is reasonable. */
typedef struct {
    const char *the_travelers_phrase;
    const char *the_babel_phrase;
    const char *her_reason;
} a_page_in_the_book;

static const a_page_in_the_book the_book_of_tongues[] = {
    /* ---- Verbs of creation ---- */
    {"make a",          "Let there be a",   "creating something new"},
    {"create a",        "Let there be a",   "creating something new"},
    {"define a",        "Let there be a",   "creating something new"},
    {"declare a",       "Let there be a",   "creating something new"},
    {"i want a",        "Let there be a",   "creating something new"},
    {"i need a",        "Let there be a",   "creating something new"},
    {"give me a",       "Let there be a",   "creating something new"},

    /* ---- Verbs of display ---- */
    {"show me",         "Print",            "displaying output"},
    {"show",            "Print",            "displaying output"},
    {"display",         "Print",            "displaying output"},
    {"tell me",         "Print",            "displaying output"},
    {"what is",         "Print",            "displaying output"},
    {"what's",          "Print",            "displaying output"},
    {"reveal",          "Print",            "displaying output"},
    {"log",             "Print",            "displaying output"},
    {"output",          "Print",            "displaying output"},
    {"write out",       "Print",            "displaying output"},
    {"spit out",        "Print",            "displaying output"},

    /* ---- Verbs of computation ---- */
    {"add up",          "the sum of",       "summing values"},
    {"add together",    "the sum of",       "summing values"},
    {"total up",        "the sum of",       "summing values"},
    {"total",           "the sum of",       "summing values"},
    {"tally",           "the sum of",       "summing values"},
    {"accumulate",      "the sum of",       "summing values"},
    {"how many",        "the length of",    "counting items"},
    {"count up",        "the length of",    "counting items"},
    {"count",           "the length of",    "counting items"},

    /* ---- Verbs of change ---- */
    {"update",          "Change",           "modifying a value"},
    {"modify",          "Change",           "modifying a value"},
    {"make it",         "Set",              "assigning a value"},
    {"assign",          "Set",              "assigning a value"},
    {"replace with",    "Change",           "modifying a value"},

    /* ---- Verbs of iteration ---- */
    {"go through",      "For every",        "iterating"},
    {"loop through",    "For every",        "iterating"},
    {"iterate over",    "For every",        "iterating"},
    {"for each",        "For every",        "iterating"},
    {"run through",     "For every",        "iterating"},
    {"step through",    "For every",        "iterating"},
    {"cycle through",   "For every",        "iterating"},
    {"check each",      "For every",        "iterating"},
    {"look at each",    "For every",        "iterating"},

    /* ---- Verbs of condition ---- */
    {"whenever",        "If",               "conditional check"},
    {"in case",         "If",               "conditional check"},
    {"assuming",        "If",               "conditional check"},
    {"when",            "If",               "conditional check"},
    {"suppose",         "If",               "conditional check"},
    {"given that",      "If",               "conditional check"},
    {"provided that",   "If",               "conditional check"},
    {"or else",         "Otherwise",        "alternative branch"},
    {"if not",          "Otherwise",        "alternative branch"},
    {"else",            "Otherwise",        "alternative branch"},

    /* ---- Comparisons ---- */
    {"above",           "greater than",     "comparison"},
    {"over",            "greater than",     "comparison"},
    {"exceeds",         "greater than",     "comparison"},
    {"more than",       "greater than",     "comparison"},
    {"higher than",     "greater than",     "comparison"},
    {"bigger than",     "greater than",     "comparison"},
    {"larger than",     "greater than",     "comparison"},
    {"below",           "less than",        "comparison"},
    {"under",           "less than",        "comparison"},
    {"fewer than",      "less than",        "comparison"},
    {"lower than",      "less than",        "comparison"},
    {"smaller than",    "less than",        "comparison"},
    {"same as",         "equals",           "equality check"},
    {"equal to",        "equals",           "equality check"},
    {"identical to",    "equals",           "equality check"},
    {"matches",         "equals",           "equality check"},

    /* ---- Type words ---- */
    {"integer",         "number",           "a numeric type"},
    {"int",             "number",           "a numeric type"},
    {"float",           "number",           "a numeric type"},
    {"decimal",         "number",           "a numeric type"},
    {"string",          "word",             "a text type"},
    {"text",            "word",             "a text type"},
    {"boolean",         "truth",            "a true/false type"},
    {"bool",            "truth",            "a true/false type"},
    {"flag",            "truth",            "a true/false type"},
    {"array",           "list",             "a collection type"},
    {"collection",      "list",             "a collection type"},

    /* ---- Number words (helpful when voices become text) ---- */
    {"zero",            "0",                "a number in words"},
    {"one",             "1",                "a number in words"},
    {"two",             "2",                "a number in words"},
    {"three",           "3",                "a number in words"},
    {"four",            "4",                "a number in words"},
    {"five",            "5",                "a number in words"},
    {"six",             "6",                "a number in words"},
    {"seven",           "7",                "a number in words"},
    {"eight",           "8",                "a number in words"},
    {"nine",            "9",                "a number in words"},
    {"ten",             "10",               "a number in words"},
    {"twenty",          "20",               "a number in words"},
    {"thirty",          "30",               "a number in words"},
    {"forty",           "40",               "a number in words"},
    {"fifty",           "50",               "a number in words"},
    {"a hundred",       "100",              "a number in words"},
    {"hundred",         "100",              "a number in words"},
    {"a thousand",      "1000",             "a number in words"},
    {"thousand",        "1000",             "a number in words"},
    {"ten thousand",    "10000",            "a number in words"},
    {"a million",       "1000000",          "a number in words"},
    {"10k",             "10000",            "a number in short form"},
    {"100k",            "100000",           "a number in short form"},
    {"1m",              "1000000",          "a number in short form"},

    {NULL, NULL, NULL}
};

/* The words the Interpreter quietly removes before she even
 * begins to listen. Grammatical filler. Throat-clearing. The
 * sounds people make while they are still deciding what they
 * really want to say. */
static const char *the_filler_words[] = {
    "okay so", "alright", "basically", "actually", "really",
    "just", "um", "uh", "you know", "i guess", "kind of",
    "sort of", "i mean", "like i said", "so yeah", "hmm",
    "please", "can you", "could you", "would you", "i want to",
    "i'd like to", "i would like to", "let's",
    NULL
};

/* How confident the Interpreter is about a single fragment of
 * speech she has tried to translate. */
typedef enum {
    UNDERSTOOD_PERFECTLY, /* the words were already Babel         */
    UNDERSTOOD_PROBABLY,  /* she translated through her book      */
    NEEDS_A_VALUE,        /* partial — she needs a missing detail */
    NOT_UNDERSTOOD        /* truly lost                           */
} how_well_she_understood;

typedef struct {
    char the_travelers_words[256];
    char the_babel_translation[256];
    how_well_she_understood her_confidence;
    char her_reason[160];
} an_interpretation;

typedef struct {
    an_interpretation fragments[64];
    int how_many_fragments;
    char the_reconstructed_babel[4096];
    int has_red_slots;
    int has_yellow_slots;
} the_interpretation;

/* A small case-insensitive match: does "needle" appear at the
 * start of "haystack"? Returns the length of the match, or 0. */
static int she_hears_phrase(const char *haystack, const char *needle) {
    int i = 0;
    while (needle[i]) {
        char a = haystack[i], b = needle[i];
        if (!a) return 0;
        if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
        if (a != b) return 0;
        i++;
    }
    /* Must be at a word boundary — the next character should not
     * be a letter, or we would match "for" inside "forty". */
    char nxt = haystack[i];
    if ((nxt >= 'a' && nxt <= 'z') || (nxt >= 'A' && nxt <= 'Z')) return 0;
    return i;
}

/* She removes the filler words first, so the rest of her
 * listening has less noise to wade through. */
static void she_removes_the_filler(const char *in, char *out, int out_size) {
    int o = 0;
    int i = 0;
    /* We keep what's inside quotation marks exactly as given. */
    int inside_quotes = 0;
    while (in[i] && o < out_size - 1) {
        if (in[i] == '"') {
            inside_quotes = !inside_quotes;
            out[o++] = in[i++];
            continue;
        }
        if (!inside_quotes) {
            char prev = (i > 0) ? in[i-1] : ' ';
            int at_word_start = !((prev >= 'a' && prev <= 'z') || (prev >= 'A' && prev <= 'Z'));
            int matched = 0;
            for (int f = 0; at_word_start && the_filler_words[f]; f++) {
                int n = she_hears_phrase(in + i, the_filler_words[f]);
                if (n > 0) {
                    i += n;
                    /* also swallow a trailing comma or space */
                    while (in[i] == ' ' || in[i] == ',') i++;
                    matched = 1;
                    break;
                }
            }
            if (matched) continue;
        }
        out[o++] = in[i++];
    }
    out[o] = '\0';
}

/* She walks the traveler's cleaned speech once, and wherever she
 * recognises a phrase from her book, she writes the Babel word
 * in its place. She prefers the longest match, so that "add up"
 * beats "add" and "show me" beats "show". She does not touch
 * anything inside quotation marks. */
static int she_consults_the_book(const char *in, char *out, int out_size) {
    int used = 0;
    int o = 0;
    int i = 0;
    int inside_quotes = 0;
    while (in[i] && o < out_size - 1) {
        if (in[i] == '"') {
            inside_quotes = !inside_quotes;
            out[o++] = in[i++];
            continue;
        }
        if (!inside_quotes) {
            /* Only start a match at a word boundary — otherwise
             * "int" would happily eat the middle of "print". */
            char prev = (i > 0) ? in[i-1] : ' ';
            int at_word_start = !((prev >= 'a' && prev <= 'z') || (prev >= 'A' && prev <= 'Z'));
            int best_n = 0;
            const a_page_in_the_book *best = NULL;
            if (at_word_start) {
                for (int p = 0; the_book_of_tongues[p].the_travelers_phrase; p++) {
                    int n = she_hears_phrase(in + i, the_book_of_tongues[p].the_travelers_phrase);
                    if (n > best_n) {
                        best_n = n;
                        best = &the_book_of_tongues[p];
                    }
                }
            }
            if (best) {
                const char *r = best->the_babel_phrase;
                while (*r && o < out_size - 1) out[o++] = *r++;
                i += best_n;
                used++;
                continue;
            }
        }
        out[o++] = in[i++];
    }
    out[o] = '\0';
    return used;
}

/* She breaks the speech at the natural seams: periods, and the
 * little connecting phrases travelers use between thoughts
 * ("and then", "then", "after that", "next"). Each seam becomes
 * one Babel sentence. */
static int she_finds_the_seams(const char *in, char segments[][512], int max) {
    static const char *connectors[] = {
        "and then", "after that", "and finally", "finally",
        "next,", "then,", "also,", NULL
    };
    int n = 0, o = 0;
    char buf[512];
    int inside_quotes = 0;
    int i = 0;
    while (in[i] && n < max) {
        if (in[i] == '"') {
            inside_quotes = !inside_quotes;
            if (o < (int)sizeof(buf) - 1) buf[o++] = in[i];
            i++;
            continue;
        }
        if (!inside_quotes) {
            int hit = 0;
            for (int c = 0; connectors[c]; c++) {
                int k = she_hears_phrase(in + i, connectors[c]);
                if (k > 0) { i += k; hit = 1; break; }
            }
            if (hit) {
                buf[o] = '\0';
                if (o > 0) {
                    strncpy(segments[n], buf, 511);
                    segments[n][511] = '\0';
                    n++;
                }
                o = 0;
                while (in[i] == ' ' || in[i] == ',' || in[i] == '.') i++;
                continue;
            }
            if (in[i] == '.') {
                buf[o] = '\0';
                if (o > 0) {
                    strncpy(segments[n], buf, 511);
                    segments[n][511] = '\0';
                    n++;
                }
                o = 0;
                i++;
                while (in[i] == ' ') i++;
                continue;
            }
        }
        if (o < (int)sizeof(buf) - 1) buf[o++] = in[i];
        i++;
    }
    if (o > 0 && n < max) {
        buf[o] = '\0';
        strncpy(segments[n], buf, 511);
        segments[n][511] = '\0';
        n++;
    }
    /* Trim trailing/leading whitespace on each segment. */
    for (int s = 0; s < n; s++) {
        char *p = segments[s];
        while (*p == ' ' || *p == '\t') p++;
        if (p != segments[s]) memmove(segments[s], p, strlen(p) + 1);
        int L = (int)strlen(segments[s]);
        while (L > 0 && (segments[s][L-1] == ' ' || segments[s][L-1] == '\t'))
            segments[s][--L] = '\0';
    }
    return n;
}

/* Ask the tower, very politely and very quietly, whether a
 * single sentence would parse as Babel. If it would, return 1.
 * If it would have made the tower apologize, catch the apology
 * and return 0. The Interpreter uses this to decide whether a
 * translated sentence is ready to run. */
static int she_asks_the_tower_quietly(const char *sentence) {
    /* Remember the pool marks so we can roll back whatever the
     * trial parse allocated. */
    int words_mark = how_many_words_so_far;
    int rooms_mark = how_many_rooms_built;
    int things_mark = how_many_things_so_far;
    int registries_mark = how_many_registries_so_far;

    /* We need a nul-terminated line ending in a newline for the
     * lexer's indentation state to settle. */
    char buf[4096];
    snprintf(buf, sizeof(buf), "%s\n", sentence);

    the_tower_is_being_patient = 1;
    the_last_quiet_apology[0] = '\0';
    int ok = 1;
    if (setjmp(the_patient_jump) == 0) {
        the_lexer she;
        the_lexer_begins(&she, buf);
        the_lexer_speaks_through_the_whole_manuscript(&she);
        (void)the_parser_builds_the_tower();
    } else {
        ok = 0;
    }
    the_tower_is_being_patient = 0;

    /* Roll back the pools so the trial does not leak. */
    how_many_words_so_far = words_mark;
    how_many_rooms_built = rooms_mark;
    how_many_things_so_far = things_mark;
    how_many_registries_so_far = registries_mark;
    return ok;
}

/* The Interpreter listens, from end to end, and hands back her
 * interpretation: what she was sure of, what she guessed, what
 * she could not make out. */
static the_interpretation the_interpreter_listens(const char *the_travelers_speech) {
    the_interpretation result;
    memset(&result, 0, sizeof(result));

    char cleaned[4096];
    she_removes_the_filler(the_travelers_speech, cleaned, sizeof(cleaned));

    char segments[64][512];
    int n = she_finds_the_seams(cleaned, segments, 64);

    for (int s = 0; s < n && result.how_many_fragments < 64; s++) {
        an_interpretation *f = &result.fragments[result.how_many_fragments];
        strncpy(f->the_travelers_words, segments[s], sizeof(f->the_travelers_words) - 1);

        /* Consult the book first — maybe the traveler used some
         * familiar-but-loose phrasing the Interpreter knows how to
         * tighten up. */
        char translated[1024];
        int used = she_consults_the_book(segments[s], translated, sizeof(translated));
        char trial[1100];
        snprintf(trial, sizeof(trial), "%s.", translated);

        /* If the book left the text alone, ask the tower whether
         * the traveler's own words are already valid Babel. The
         * tower is a strict judge — it will reject references to
         * names it has not yet met — but when it says yes, we can
         * be sure. */
        if (used == 0 && she_asks_the_tower_quietly(trial)) {
            strncpy(f->the_babel_translation, trial, sizeof(f->the_babel_translation) - 1);
            f->her_confidence = UNDERSTOOD_PERFECTLY;
            strncpy(f->her_reason, "already valid Babel", sizeof(f->her_reason) - 1);
            result.how_many_fragments++;
            continue;
        }

        /* The book did something, or the parser was unhappy. We
         * take the translation as our best guess and mark it
         * yellow — the traveler can say "yes" to confirm. */
        strncpy(f->the_babel_translation, trial, sizeof(f->the_babel_translation) - 1);
        if (used > 0) {
            f->her_confidence = UNDERSTOOD_PROBABLY;
            snprintf(f->her_reason, sizeof(f->her_reason),
                     "I recognised %d familiar phrase%s", used, used == 1 ? "" : "s");
        } else {
            f->her_confidence = UNDERSTOOD_PROBABLY;
            strncpy(f->her_reason,
                    "I'm passing this through as-is — does it look right?",
                    sizeof(f->her_reason) - 1);
        }
        result.has_yellow_slots = 1;
        result.how_many_fragments++;
    }

    /* Assemble whatever we have so far. */
    result.the_reconstructed_babel[0] = '\0';
    int rb = 0;
    for (int i = 0; i < result.how_many_fragments; i++) {
        const char *t = result.fragments[i].the_babel_translation;
        if (!t[0]) continue;
        int L = (int)strlen(t);
        if (rb + L + 2 >= (int)sizeof(result.the_reconstructed_babel)) break;
        memcpy(result.the_reconstructed_babel + rb, t, L);
        rb += L;
        result.the_reconstructed_babel[rb++] = '\n';
    }
    result.the_reconstructed_babel[rb] = '\0';
    return result;
}

/* A small, cheerful vocabulary of yeses. No NLP, just a list. */
static int she_hears_a_yes(const char *response) {
    static const char *yeses[] = {
        "yes", "y", "yeah", "yep", "yup", "correct", "right",
        "ok", "okay", "sure", "mhm", "yea", "absolutely",
        "definitely", "that's right", "exactly", "perfect",
        "confirmed", "go", "go ahead", "do it", "run it",
        "proceed", "ship it", "send it", NULL
    };
    /* Lowercase a local copy, trimming whitespace. */
    char lower[128];
    int o = 0, i = 0;
    while (response[i] == ' ' || response[i] == '\t') i++;
    while (response[i] && o < (int)sizeof(lower) - 1) {
        char c = response[i++];
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
        lower[o++] = c;
    }
    while (o > 0 && (lower[o-1] == ' ' || lower[o-1] == '\n' || lower[o-1] == '\r' || lower[o-1] == '.' || lower[o-1] == '!'))
        o--;
    lower[o] = '\0';
    for (int k = 0; yeses[k]; k++)
        if (strcmp(lower, yeses[k]) == 0) return 1;
    return 0;
}

/* She shows the traveler her understanding, all at once, and
 * marks each piece with a small sign: a check for the parts she
 * is sure of, a question mark for the parts she guessed, a cross
 * for the parts she could not make out. */
#define THE_GREEN  "\x1b[32m"
#define THE_YELLOW "\x1b[33m"
#define THE_RED    "\x1b[31m"
#define THE_DIM    "\x1b[2m"
#define THE_BOLD   "\x1b[1m"
#define THE_RESET  "\x1b[0m"

static void she_shows_her_understanding(const the_interpretation *interp) {
    printf("\n  I think you mean:\n\n");
    for (int i = 0; i < interp->how_many_fragments; i++) {
        const an_interpretation *f = &interp->fragments[i];
        const char *mark;
        const char *color;
        switch (f->her_confidence) {
        case UNDERSTOOD_PERFECTLY: mark = "✓"; color = THE_GREEN;  break;
        case UNDERSTOOD_PROBABLY:  mark = "?"; color = THE_YELLOW; break;
        case NEEDS_A_VALUE:        mark = "…"; color = THE_YELLOW; break;
        default:                   mark = "✗"; color = THE_RED;    break;
        }
        if (f->the_babel_translation[0]) {
            printf("    %s%s%s %s%s%s\n",
                   color, mark, THE_RESET,
                   THE_BOLD, f->the_babel_translation, THE_RESET);
        } else {
            printf("    %s%s%s \"%s\"\n",
                   color, mark, THE_RESET, f->the_travelers_words);
        }
        if (f->her_confidence != UNDERSTOOD_PERFECTLY)
            printf("      %s← %s%s\n", THE_DIM, f->her_reason, THE_RESET);
    }
    printf("\n");
}

/* When there are yellow slots (guesses) or red slots (unknowns),
 * she walks the traveler through them one at a time. A yes
 * confirms; a correction replaces; "skip" drops the fragment. */
static void she_resolves_the_uncertain(the_interpretation *interp) {
    if (interp->has_yellow_slots) {
        printf("  Quick confirmations — say %syes%s, or correct me:\n\n",
               THE_BOLD, THE_RESET);
        for (int i = 0; i < interp->how_many_fragments; i++) {
            an_interpretation *f = &interp->fragments[i];
            if (f->her_confidence != UNDERSTOOD_PROBABLY) continue;
            printf("    %s?%s \"%s\"\n", THE_YELLOW, THE_RESET, f->the_travelers_words);
            printf("      → %s%s%s\n", THE_BOLD, f->the_babel_translation, THE_RESET);
            printf("      correct? ");
            fflush(stdout);
            char reply[256];
            if (!fgets(reply, sizeof(reply), stdin)) return;
            reply[strcspn(reply, "\n")] = '\0';
            if (reply[0] == '\0' || she_hears_a_yes(reply)) {
                f->her_confidence = UNDERSTOOD_PERFECTLY;
                printf("      %s✓%s\n\n", THE_GREEN, THE_RESET);
                continue;
            }
            /* The traveler gave a correction. Try it as Babel, or
             * translate through the book. */
            char with_period[300];
            snprintf(with_period, sizeof(with_period), "%s.", reply);
            if (she_asks_the_tower_quietly(with_period)) {
                strncpy(f->the_babel_translation, with_period, sizeof(f->the_babel_translation) - 1);
                f->her_confidence = UNDERSTOOD_PERFECTLY;
                printf("      %s✓ Updated%s\n\n", THE_GREEN, THE_RESET);
                continue;
            }
            char retranslated[512];
            int used = she_consults_the_book(reply, retranslated, sizeof(retranslated));
            char with_period2[600];
            snprintf(with_period2, sizeof(with_period2), "%s.", retranslated);
            if (used > 0 && she_asks_the_tower_quietly(with_period2)) {
                strncpy(f->the_babel_translation, with_period2, sizeof(f->the_babel_translation) - 1);
                f->her_confidence = UNDERSTOOD_PERFECTLY;
                printf("      %s✓ Got it: %s%s\n\n", THE_GREEN, with_period2, THE_RESET);
            } else {
                printf("      %s✗ I still don't understand — skipping.%s\n\n", THE_RED, THE_RESET);
                f->her_confidence = NOT_UNDERSTOOD;
                f->the_babel_translation[0] = '\0';
            }
        }
    }

    if (interp->has_red_slots) {
        printf("  I need your help with a few things:\n\n");
        for (int i = 0; i < interp->how_many_fragments; i++) {
            an_interpretation *f = &interp->fragments[i];
            if (f->her_confidence == UNDERSTOOD_PERFECTLY) continue;
            if (f->her_confidence == UNDERSTOOD_PROBABLY) continue;
            printf("    you said: \"%s\"\n", f->the_travelers_words);
            if (f->her_confidence == NEEDS_A_VALUE)
                printf("    I got: %s%s%s, but a piece is missing.\n",
                       THE_BOLD, f->the_babel_translation, THE_RESET);
            else
                printf("    I don't know how to put this into Babel.\n");
            printf("    say it differently, or type %sskip%s: ", THE_BOLD, THE_RESET);
            fflush(stdout);
            char reply[256];
            if (!fgets(reply, sizeof(reply), stdin)) return;
            reply[strcspn(reply, "\n")] = '\0';
            if (strcmp(reply, "skip") == 0 || strcmp(reply, "nevermind") == 0 || reply[0] == '\0') {
                f->the_babel_translation[0] = '\0';
                f->her_confidence = UNDERSTOOD_PERFECTLY;
                printf("    %sskipped%s\n\n", THE_DIM, THE_RESET);
                continue;
            }
            char with_period[300];
            snprintf(with_period, sizeof(with_period), "%s.", reply);
            if (she_asks_the_tower_quietly(with_period)) {
                strncpy(f->the_babel_translation, with_period, sizeof(f->the_babel_translation) - 1);
                f->her_confidence = UNDERSTOOD_PERFECTLY;
                printf("    %s✓%s\n\n", THE_GREEN, THE_RESET);
                continue;
            }
            char retranslated[512];
            int used = she_consults_the_book(reply, retranslated, sizeof(retranslated));
            char with_period2[600];
            snprintf(with_period2, sizeof(with_period2), "%s.", retranslated);
            if (used > 0 && she_asks_the_tower_quietly(with_period2)) {
                strncpy(f->the_babel_translation, with_period2, sizeof(f->the_babel_translation) - 1);
                f->her_confidence = UNDERSTOOD_PERFECTLY;
                printf("    %s✓ Got it: %s%s\n\n", THE_GREEN, with_period2, THE_RESET);
            } else {
                printf("    %s✗ still lost — skipping.%s\n\n", THE_RED, THE_RESET);
                f->the_babel_translation[0] = '\0';
                f->her_confidence = NOT_UNDERSTOOD;
            }
        }
    }

    /* Reassemble after the corrections. */
    interp->the_reconstructed_babel[0] = '\0';
    int rb = 0;
    for (int i = 0; i < interp->how_many_fragments; i++) {
        const char *t = interp->fragments[i].the_babel_translation;
        if (!t[0]) continue;
        int L = (int)strlen(t);
        if (rb + L + 2 >= (int)sizeof(interp->the_reconstructed_babel)) break;
        memcpy(interp->the_reconstructed_babel + rb, t, L);
        rb += L;
        interp->the_reconstructed_babel[rb++] = '\n';
    }
    interp->the_reconstructed_babel[rb] = '\0';
}

/* ================================================================
 * EPILOGUE: THE TOWER STANDS
 *
 * The tower was never finished. That was the point. Each new
 * speaker who arrived added their own rooms, their own names,
 * their own meanings. The Lexer kept finding new words. The
 * Parser kept building new structures. The Evaluator kept
 * discovering new truths.
 *
 * They had set out to build a tower that reached heaven.
 * What they built instead was a language.
 *
 * And that, perhaps, was the same thing.
 * ================================================================ */

/* A small REPL. Reads from stdin until "Goodbye." or end-of-file. */
static void the_conversation_begins(void) {
    printf("\n");
    printf("  +======================================+\n");
    printf("  |             B A B E L                |\n");
    printf("  |   speak, and the tower will answer   |\n");
    printf("  +--------------------------------------+\n");
    printf("  |  Say \"Goodbye.\" when you are done.   |\n");
    printf("  +======================================+\n\n");

    the_registry *foundation = a_fresh_registry(NULL);
    char buffer[8192];
    while (1) {
        printf("babel> ");
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        if (strstr(buffer, "Goodbye") || strstr(buffer, "goodbye")) break;

        /* Reset the lexer/parser/evaluator pools so each line is fresh
         * but the registry persists across the conversation. */
        the_lexer she;
        the_lexer_begins(&she, buffer);
        the_lexer_speaks_through_the_whole_manuscript(&she);
        the_room *plan = the_parser_builds_the_tower();
        the_thing *result = the_evaluator_enters(plan, foundation);
        if (result && result->what_it_is != THING_NOTHING) {
            the_evaluator_speaks_a_thing(result, ", ");
            printf("\n");
        }
    }
    printf("\nThe tower stands. Goodbye.\n");
}

/* A second kind of conversation — this one runs every line past
 * the Interpreter of Tongues first. The traveler may speak however
 * they like; the Interpreter shows what she heard, asks for any
 * confirmations she needs, and then hands the agreed-upon Babel
 * to the tower. */
static void the_conversation_begins_with_an_interpreter(void) {
    printf("\n");
    printf("  +==========================================+\n");
    printf("  |             B A B E L                    |\n");
    printf("  |   speak however you like — the Interpreter|\n");
    printf("  |   will translate for the tower.          |\n");
    printf("  +------------------------------------------+\n");
    printf("  |  Say \"Goodbye.\" when you are done.       |\n");
    printf("  +==========================================+\n\n");

    the_registry *foundation = a_fresh_registry(NULL);
    char buffer[8192];
    while (1) {
        printf("you> ");
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        if (strstr(buffer, "Goodbye") || strstr(buffer, "goodbye")) break;
        buffer[strcspn(buffer, "\n")] = '\0';
        if (buffer[0] == '\0') continue;

        the_interpretation interp = the_interpreter_listens(buffer);
        if (interp.how_many_fragments == 0) {
            printf("  (I heard only silence.)\n\n");
            continue;
        }
        she_shows_her_understanding(&interp);
        if (interp.has_yellow_slots || interp.has_red_slots)
            she_resolves_the_uncertain(&interp);

        if (interp.the_reconstructed_babel[0] == '\0') {
            printf("  (nothing left to run)\n\n");
            continue;
        }

        /* Hand the agreed-upon Babel to the tower. */
        the_lexer she;
        the_lexer_begins(&she, interp.the_reconstructed_babel);
        the_lexer_speaks_through_the_whole_manuscript(&she);
        the_room *plan = the_parser_builds_the_tower();
        the_thing *result = the_evaluator_enters(plan, foundation);
        if (result && result->what_it_is != THING_NOTHING) {
            the_evaluator_speaks_a_thing(result, ", ");
            printf("\n");
        }
    }
    printf("\nThe tower stands. Goodbye.\n");
}

/* The story begins, as all stories must, with someone speaking. */
int main(int the_count_of_arguments, char **the_arguments_themselves) {
    the_world_is_made();

    /* The Scribe is summoned only when asked.
     *   -c [-o out]   — write a native binary through the C twin
     *   -p [-o out]   — write a Python transcription instead
     * Otherwise the Evaluator does her usual quiet walk. */
    int the_scribe_is_called = 0;
    int the_scribe_speaks_python = 0;
    int the_interpreter_is_called = 0;
    const char *the_output_binary = "a.out";
    const char *the_output_python = NULL;
    int first_arg = 1;
    while (first_arg < the_count_of_arguments && the_arguments_themselves[first_arg][0] == '-') {
        if (strcmp(the_arguments_themselves[first_arg], "-i") == 0) {
            the_interpreter_is_called = 1;
            first_arg++;
        } else if (strcmp(the_arguments_themselves[first_arg], "-c") == 0) {
            the_scribe_is_called = 1;
            first_arg++;
            if (first_arg < the_count_of_arguments &&
                strcmp(the_arguments_themselves[first_arg], "-o") == 0) {
                first_arg++;
                if (first_arg < the_count_of_arguments)
                    the_output_binary = the_arguments_themselves[first_arg++];
            }
        } else if (strcmp(the_arguments_themselves[first_arg], "-p") == 0) {
            the_scribe_speaks_python = 1;
            first_arg++;
            if (first_arg < the_count_of_arguments &&
                strcmp(the_arguments_themselves[first_arg], "-o") == 0) {
                first_arg++;
                if (first_arg < the_count_of_arguments)
                    the_output_python = the_arguments_themselves[first_arg++];
            }
        } else break;
    }

    if (first_arg < the_count_of_arguments) {
        const char *the_manuscript_path = the_arguments_themselves[first_arg];
        FILE *the_manuscript = fopen(the_manuscript_path, "rb");
        if (!the_manuscript) {
            fprintf(stderr,
                "I looked for a manuscript called \"%s\" but could not find it.\n"
                "Perhaps it is in another room?\n",
                the_manuscript_path);
            return 1;
        }
        fseek(the_manuscript, 0, SEEK_END);
        long its_length = ftell(the_manuscript);
        fseek(the_manuscript, 0, SEEK_SET);
        char *the_speech = (char*)malloc(its_length + 2);
        size_t how_much_was_read = fread(the_speech, 1, its_length, the_manuscript);
        the_speech[how_much_was_read] = '\n';
        the_speech[how_much_was_read + 1] = '\0';
        fclose(the_manuscript);

        /* If the Interpreter has been summoned for a file, we run
         * the whole manuscript through her first, then hand the
         * translated Babel to the tower. Corrections are impossible
         * without a human at the keyboard, so yellow slots are
         * accepted as-is and red slots are dropped with a warning. */
        char *the_speech_to_parse = the_speech;
        the_interpretation file_interp;
        if (the_interpreter_is_called) {
            file_interp = the_interpreter_listens(the_speech);
            if (file_interp.has_red_slots) {
                fprintf(stderr,
                    "The Interpreter could not translate every line of \"%s\".\n"
                    "Running what she understood; the rest was dropped.\n",
                    the_manuscript_path);
            }
            the_speech_to_parse = file_interp.the_reconstructed_babel;
        }

        the_lexer she;
        the_lexer_begins(&she, the_speech_to_parse);
        the_lexer_speaks_through_the_whole_manuscript(&she);

        the_room *the_tower = the_parser_builds_the_tower();

        if (the_scribe_is_called) {
            int rc = the_scribe_compiles(the_tower, the_output_binary);
            free(the_speech);
            return rc;
        }

        if (the_scribe_speaks_python) {
            int rc = the_scribe_transcribes_into_python(the_tower, the_output_python);
            free(the_speech);
            return rc;
        }

        the_registry *the_foundation = a_fresh_registry(NULL);
        the_evaluator_enters(the_tower, the_foundation);

        free(the_speech);
        return 0;
    }

    if (the_interpreter_is_called)
        the_conversation_begins_with_an_interpreter();
    else
        the_conversation_begins();
    return 0;
}

/*
 * And so the tower stands. Each name has been given a place to
 * live. Each loop has run its course. Each answer has been
 * carried home. The Lexer has gone back to her search for words.
 * The Parser has gone back to his drawings. The Evaluator has
 * gone back to her quiet walks through finished rooms.
 *
 * What they built together was simple, in the end:
 * a way to say something, and have it heard.
 */
