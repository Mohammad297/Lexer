from typing import Any

from tqdm import trange, tqdm
from random import choice, random, randint
from pathlib import Path
from sys import argv
from collections.abc import Callable


########################################################################################################

if len(argv) < 3:
    print("syntax: <target-file> <token-count>")
    exit(1)

if not argv[2].isdecimal():
    print("argument 2: token count must be a number")
    exit(2)

########################################################################################################

var_keywords = ["var", "const", "static"]
stt_keywords = ["if", "while", "else", "switch", "case", "break", "continue", "default", "for"]
keywords = var_keywords + stt_keywords
typenames = ["int", "uint", "char", "string", "bool"]
literals = ["true", "false"]
identifiers = keywords + typenames + literals

uni_operators = ["++", "--", "+", "-", "!", "~"]
bin_operators = ["+", "-", "*", "/", "%", "^", "<<", ">>", "|", "&"]
asn_operators = ["="] + list(map(lambda op: op + "=", bin_operators))
cmp_operators = ["==", "<", ">", "<=", ">=", "!="]
operators = uni_operators + bin_operators + asn_operators + cmp_operators

brackets = ["()", "[]", "{}"]
separators = [",", ".", ":", ";"]
delimiters = [bracket for pair in brackets for bracket in pair] + separators

identifierHeads = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
identifierTails = identifierHeads + "0123456789"

########################################################################################################

type StatementOutput = tuple[str, int]
'''
Contains: [generated statement string, number of tokens]
'''

type StatementFunction = Callable[..., StatementOutput]

type TokenLimiter = Callable[[int], int]
'''
Params: int parameter affecting the final number of tokens
Returns: the final maximum number of tokens
'''

type StatementHandle = tuple[tuple[Any, ...], StatementFunction, TokenLimiter]
'''
Contains: [arguments to pass, statement-generating function to call, function returning the max number of tokens the StatementFunction may generate]
'''

skips: TokenLimiter = lambda _: 0
singleToken: TokenLimiter = lambda _: 1
sameTokens: TokenLimiter = lambda n: n
modifiedIdentifierTokens: TokenLimiter = lambda _: 3
joinedTokens: TokenLimiter = lambda n: n * 2 - 1

handles: list[StatementHandle] = []

def registerHandler(args: tuple[Any, ...], func: StatementFunction, limiter: TokenLimiter):
    if any(h[0] == func for h in handles):
        return False
    handles.append((args, func, limiter))
    return True

########################################################################################################


#max tokens returned: 1
def newIdentifier(minLength: int, maxLength: int):
    length = randint(minLength, maxLength)
    return choice(identifierHeads) + ''.join(choice(identifierTails) for _ in range(length)), 1
registerHandler((2, 4), newIdentifier, singleToken)

#max tokens returned: 1
def newLiteral(reservedLiterals: list[str], odds: float):
    if random() < odds:
        return choice(reservedLiterals), 1
    else:
        roll = random()
        if roll < 0.2:  return str(randint(1, 5)), 1
        elif roll < 0.4: return choice("-+") + str(randint(1, 4)), 1
        elif roll < 0.6: return choice("-+ ") + str(randint(1, 2)) + "." + str(randint(1, 2)), 1
        elif roll < 0.8:
            if random() < 0.5:
                return f"'{choice(identifierTails)}'", 1
            else:
                return f"'\\{choice(identifierTails)}'", 1
        else:
            identifier, _ = newIdentifier(2, 7)
            return '"' + identifier + '"', 1
registerHandler((literals, 0.1), newLiteral, singleToken)
        
#max tokens returned: 1
def newValue(identifierSet: list[str], odds: float, reservedLiterals: list[str], literalInternalOdds: float):
    if random() < odds: return choice(identifierSet), 1
    else: return newLiteral(reservedLiterals, literalInternalOdds)
registerHandler((identifiers, 0.5, literals, 0.1), newValue, singleToken)

#max tokens returned: termCount * 2 - 1
def newTerms(identifierSet: list[str], reservedLiterals: list[str], oepratorSet: list[str], termCount: int = 2, identifierOdds: float = 0.1, literalOdds: float = 0.5):
    return f' {choice(oepratorSet)} '.join(newValue(identifierSet, identifierOdds, reservedLiterals, literalOdds)[0] for _ in range(termCount)), termCount * 2 - 1
registerHandler((identifiers, literals, operators, 3, 0.1, 0.5), newTerms, joinedTokens)

#max tokens returned: 3
def newAssignmentTarget(identifierSet: list[str], typenames: list[str], typeOdds: float, modifierOdds: float):
    target = ""
    count = 1
    if random() < modifierOdds:
        target += choice(var_keywords)
        count += 1
    if random() < typeOdds:
        target += f" {choice(typenames)}"
        count += 1
    target += f" {choice(identifierSet)}"
    return target, count
registerHandler((identifiers, typenames, 0.5, 0.5), newAssignmentTarget, modifiedIdentifierTokens)

#max tokens returned: maxTerms + 4
def newAssignment(identifierSet: list[str], typenames: list[str], typeOdds: float, modifierOdds: float, odds: float, maxTerms: int = 1):
    target, targetCount = newAssignmentTarget(identifierSet, typenames, typeOdds, modifierOdds)
    assignment = choice(asn_operators)
    if maxTerms < 1:
        return f"{target} {assignment} {choice(identifierSet)}", targetCount + 2
    
    if random() < odds:
        return f"{target} {assignment} {choice(identifierSet)}", targetCount + 2
    else:
        termCount = randint(1, maxTerms)
        terms, count = newTerms(identifierSet, literals, operators, termCount)
        return f"{target} {assignment} {terms}", targetCount + 1 + termCount
registerHandler((identifiers, typenames, 0.5, 0.5, 0.5, 3), newAssignment, lambda maxTerms: maxTerms + 4)
    
#max tokens returned: 0, only comments
def newSingleLineComment(minLength: int, maxLength: int, newlineOdds: float):
    if random() < newlineOdds:
        return '#' + 'c' * randint(minLength, maxLength) + '\n', 0
    else:
        return '#' + 'c' * randint(minLength, maxLength), 0
registerHandler((5, 12, 0.5), newSingleLineComment, skips)
    
#max tokens returned: 0, only comments
def newBlockComment(minLineLength: int, maxLineLength: int, minLineCount: int, maxLineCount: int, newlinePaddingOdds: float):
    doPadding = random() < newlinePaddingOdds
    comment = "#-"

    lineCount = range(randint(minLineCount, maxLineCount))
    if lineCount == 0:
        return comment, 0

    if doPadding:
        comment += "\n"

    for _ in lineCount:
        comment += 'C' * randint(minLineLength, maxLineLength)

    comment += "-#"

    if doPadding:
        comment += "\n"

    return comment, 0
registerHandler((2, 6, 4, 15, 0.7), newBlockComment, skips)

#max tokens returned: keywordCount
def newKeywordNonsense(keywordCount: int):
    return ' '.join(choice(keywords) for i in range(keywordCount)) + '\n', keywordCount
registerHandler((4,), newKeywordNonsense, sameTokens)

#max tokens returned: identifierCount
def newIdentifierNonsense(minIdentifierLength: int, maxIdentifierLength: int, identifierCount: int):
    return ' '.join(newIdentifier(minIdentifierLength, maxIdentifierLength)[0] for _ in range(identifierCount)) + '\n', identifierCount
registerHandler((3, 5, 4), newIdentifierNonsense, sameTokens)

########################################################################################################

count = int(argv[2])
path = Path(argv[1])
'''
if not path.exists():
    print("argument 1: target file does not exist")
    exit(3)
    '''

total_tokens = 0
with path.open('w+') as f:
    pbar = trange(count, total=count, desc="Writing tokens", unit="tokens")
    iterator = iter(pbar)
    
    while total_tokens < count:
        args, func, limiter = choice(handles)

        content, tokens_generated = func(*args)

        f.write(content + " ")

        if random() < 0.1:
            f.write("\n" * randint(1, 4))

        total_tokens += tokens_generated
        pbar.update(tokens_generated)
    
    pbar.close()
