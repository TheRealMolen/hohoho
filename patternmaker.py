# random christmas colour pattern maker
import random


RED = 0
GREEN = 1
BLUE = 2
AMBER = 3
PINK = 4

PALETTE = [RED, GREEN, BLUE, AMBER]
PALETTE_STR = ['Red', 'Green', 'Blue', 'Amber']

if False:
    PALETTE.append(PINK)
    PALETTE_STR.append('Pink')


CLASHES = [ (RED,PINK) ]

PATTERNLEN = 263


def is_valid_col(col, pattern):
    if len(pattern) == 0:
        return True

    final = pattern[-1]
    if final == col:
        return False

    if (final,col) in CLASHES or (col,final) in CLASHES:
        return False
    
    # avoid overly repetitive blocks
    tail = pattern[:-10]
    repeats = 0
    for c in tail:
        if c == col:
            repeats += 1
    if repeats > len(tail) * 3:
        return False

    return True


def gen_pattern():
    pat = []
    
    while len(pat) < PATTERNLEN:
        col = random.choice(PALETTE)
        if is_valid_col(col, pat):
            pat.append(col)
    return pat


def chunked_list(lst, chunk_len):
    for offset in range(0, len(lst), chunk_len):
        yield lst[offset : offset+chunk_len]

def gen_code(pat):

    print('enum EPalette : unsigned char {')
    print(f'  Pal_Red={RED},')
    print(f'  Pal_Green={GREEN},')
    print(f'  Pal_Blue={BLUE},')
    print(f'  Pal_Amber={AMBER},')
    print(f'  Pal_COUNT={len(PALETTE)},')
    print('};')
    print('')

    chunks = chunked_list(pat, 12)
    print('static constexpr EPalette kXmasPal[] = {')
    for chunk in chunks:
        print(f'  ' + ' '.join([f'Pal_{PALETTE_STR[c]},' for c in chunk]))
    print('};')
    print(f'static constexpr int kXmasPalLen = sizeof(kXmasPal);')
    print(f'static_assert(kXmasPalLen == {PATTERNLEN});')


gen_code(gen_pattern())
