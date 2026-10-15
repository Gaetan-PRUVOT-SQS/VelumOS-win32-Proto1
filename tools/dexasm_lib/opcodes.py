"""Table des instructions Dalvik : nom, code, format, genre d'indice."""

TAILLES = {
    "10x": 1,
    "12x": 1,
    "11n": 1,
    "11x": 1,
    "10t": 1,
    "20t": 2,
    "22x": 2,
    "21t": 2,
    "21s": 2,
    "21h": 2,
    "21c": 2,
    "23x": 2,
    "22b": 2,
    "22t": 2,
    "22s": 2,
    "22c": 2,
    "30t": 3,
    "32x": 3,
    "31i": 3,
    "31t": 3,
    "31c": 3,
    "35c": 3,
    "3rc": 3,
    "51l": 5,
}

TYPES_TABLEAU = ("", "-wide", "-object", "-boolean", "-byte", "-char", "-short")
OPS_INT = ("add", "sub", "mul", "div", "rem", "and", "or", "xor", "shl", "shr", "ushr")
OPS_FLOTTANT = ("add", "sub", "mul", "div", "rem")
OPS_LIT = ("add", "rsub", "mul", "div", "rem", "and", "or", "xor", "shl", "shr", "ushr")
INVOKES = ("virtual", "super", "direct", "static", "interface")
CONVERSIONS = (
    "neg-int not-int neg-long not-long neg-float neg-double int-to-long int-to-float "
    "int-to-double long-to-int long-to-float long-to-double float-to-int float-to-long "
    "float-to-double double-to-int double-to-long double-to-float int-to-byte int-to-char "
    "int-to-short"
).split()
FIXES = (
    (0x00, "nop", "10x", ""),
    (0x01, "move", "12x", ""),
    (0x02, "move/from16", "22x", ""),
    (0x03, "move/16", "32x", ""),
    (0x04, "move-wide", "12x", ""),
    (0x05, "move-wide/from16", "22x", ""),
    (0x06, "move-wide/16", "32x", ""),
    (0x07, "move-object", "12x", ""),
    (0x08, "move-object/from16", "22x", ""),
    (0x09, "move-object/16", "32x", ""),
    (0x0A, "move-result", "11x", ""),
    (0x0B, "move-result-wide", "11x", ""),
    (0x0C, "move-result-object", "11x", ""),
    (0x0D, "move-exception", "11x", ""),
    (0x0E, "return-void", "10x", ""),
    (0x0F, "return", "11x", ""),
    (0x10, "return-wide", "11x", ""),
    (0x11, "return-object", "11x", ""),
    (0x12, "const/4", "11n", ""),
    (0x13, "const/16", "21s", ""),
    (0x14, "const", "31i", ""),
    (0x15, "const/high16", "21h", ""),
    (0x16, "const-wide/16", "21s", ""),
    (0x17, "const-wide/32", "31i", ""),
    (0x18, "const-wide", "51l", ""),
    (0x19, "const-wide/high16", "21h", ""),
    (0x1A, "const-string", "21c", "chaine"),
    (0x1B, "const-string/jumbo", "31c", "chaine"),
    (0x1C, "const-class", "21c", "type"),
    (0x1D, "monitor-enter", "11x", ""),
    (0x1E, "monitor-exit", "11x", ""),
    (0x1F, "check-cast", "21c", "type"),
    (0x20, "instance-of", "22c", "type"),
    (0x21, "array-length", "12x", ""),
    (0x22, "new-instance", "21c", "type"),
    (0x23, "new-array", "22c", "type"),
    (0x24, "filled-new-array", "35c", "type"),
    (0x25, "filled-new-array/range", "3rc", "type"),
    (0x26, "fill-array-data", "31t", ""),
    (0x27, "throw", "11x", ""),
    (0x28, "goto", "10t", ""),
    (0x29, "goto/16", "20t", ""),
    (0x2A, "goto/32", "30t", ""),
    (0x2B, "packed-switch", "31t", ""),
    (0x2C, "sparse-switch", "31t", ""),
    (0x2D, "cmpl-float", "23x", ""),
    (0x2E, "cmpg-float", "23x", ""),
    (0x2F, "cmpl-double", "23x", ""),
    (0x30, "cmpg-double", "23x", ""),
    (0x31, "cmp-long", "23x", ""),
)


def _construire():
    table = {nom: (code, fmt, genre) for code, nom, fmt, genre in FIXES}
    for i, test in enumerate(("eq", "ne", "lt", "ge", "gt", "le")):
        table[f"if-{test}"] = (0x32 + i, "22t", "")
        table[f"if-{test}z"] = (0x38 + i, "21t", "")
    for i, suffixe in enumerate(TYPES_TABLEAU):
        table[f"aget{suffixe}"] = (0x44 + i, "23x", "")
        table[f"aput{suffixe}"] = (0x4B + i, "23x", "")
        table[f"iget{suffixe}"] = (0x52 + i, "22c", "champ")
        table[f"iput{suffixe}"] = (0x59 + i, "22c", "champ")
        table[f"sget{suffixe}"] = (0x60 + i, "21c", "champ")
        table[f"sput{suffixe}"] = (0x67 + i, "21c", "champ")
    for i, genre in enumerate(INVOKES):
        table[f"invoke-{genre}"] = (0x6E + i, "35c", "methode")
        table[f"invoke-{genre}/range"] = (0x74 + i, "3rc", "methode")
    for i, nom in enumerate(CONVERSIONS):
        table[nom] = (0x7B + i, "12x", "")
    binaires = [f"{op}-int" for op in OPS_INT] + [f"{op}-long" for op in OPS_INT]
    binaires += [f"{op}-float" for op in OPS_FLOTTANT] + [f"{op}-double" for op in OPS_FLOTTANT]
    for i, nom in enumerate(binaires):
        table[nom] = (0x90 + i, "23x", "")
        table[f"{nom}/2addr"] = (0xB0 + i, "12x", "")
    for i, op in enumerate(OPS_LIT[:8]):
        nom = "rsub-int" if op == "rsub" else f"{op}-int/lit16"
        table[nom] = (0xD0 + i, "22s", "")
    for i, op in enumerate(OPS_LIT):
        table[f"{op}-int/lit8"] = (0xD8 + i, "22b", "")
    return table


INSTRUCTIONS = _construire()
PAR_CODE = {code: (nom, fmt, genre) for nom, (code, fmt, genre) in INSTRUCTIONS.items()}
