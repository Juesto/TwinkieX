import argparse
from glob import glob
from enum import Enum
from collections import defaultdict
import json
import os
import traceback
from pathlib import Path
import datetime
import shutil
is_in_github_actions = os.getenv("GITHUB_ACTIONS") == "true"

try:
    from htbuilder import html, head, meta, link, title, body, h2, a, div, ul, li, code, p, span
except ModuleNotFoundError:
    if is_in_github_actions: quit(0)

def pluralize(string: str):
    return (string + "s" if string[-1] != "y" else string[:-1] + "ies") if string[-1] != "s" else string + "es"

class BeingInfo(Enum):
    Function = "function"
    Behavior = "behavior"
    Method = "method"
    Prop = "prop"
    Enum = "enum"
    Funcdef = "funcdef"
    Global = "global"

class DeclInfo(Enum):
    Full = "full"
    Partial = "partial"
    Deleted = "deleted"
    NoImpl = "noimpl"
    Undefined = "undefined"

class CompatInfo:
    def __init__(self):
        self.namespace = ""
        self.wanted: list[str] = []
        self.impl = ""
        self.being = ""
        self.decl: DeclInfo = DeclInfo.Undefined
        self.reason = ""
        self.of = ""
        self.games = ["TMCN", "TMU", "ESWC", "TMSX", "TMO", "TMS", "TM1"]
        self.since = ""

        self.line = 0
        self.file = ""

        self.matched = False

    def From(self, command: str, arg: str):
        match command:
            case "for":
                self.namespace = arg
            case "with":
                self.wanted.append(arg)
            case "as":
                self.impl = arg
            case "being":
                if arg not in BeingInfo:
                    raise SyntaxError(f"{arg} is not a valid type ({self.__repr__()})")
                self.being = arg
            case "of":
                self.of = arg
            case "declare":
                if arg not in DeclInfo:
                    raise SyntaxError(f"{arg} is not a valid declaration ({self.__repr__()})")
                self.decl = DeclInfo(arg)
            case "games":
                self.games = arg.split(", ")   
            case "since":
                self.since = arg         
            case "reason":
                self.reason = arg
            case _: pass

    def ToHtml(self):
        meta_items = []
        if self.namespace:
            meta_items.append(p(_class="meta-item")(f"Namespace: {self.namespace}"))
        if self.being:
            meta_items.append(p(_class="meta-item")(f"Type: {self.being}"))
        if self.of:
            meta_items.append(p(_class="meta-item")(f"Of: {self.of}"))
        if self.since:
            meta_items.append(p(_class="meta-item")(f"Since: {self.since}"))
        if self.games:
            meta_items.append(p(_class="meta-item")(f"Games: {', '.join(self.games)}"))

        return div(_class="compat-card")(
            ul(
                [li(
                    a(href=f"https://github.com/TwinkieTweaks/TwinkieX/tree/development/{self.file}#L{self.line}")(
                        code(wanted)
                    ),
                    span(style="padding-left: 5px;", _class=f"{self.decl.value}")(
                        self.decl.value
                    )
                ) for wanted in self.wanted]
            ),
            div(_class="compat-meta", style="font-size: 0.9em; color: lightgrey; margin: 5px 0;")(meta_items) if meta_items else "",
            p(self.reason + (f" (implemented as <code class=\"inline-code\">{self.impl}</code>)" if self.impl not in self.wanted else "") if len(self.reason) else "<i>No reason provided</i>.") if self.decl != DeclInfo.Full else ""
        )

    def __eq__(self, other: object):
        if not isinstance(other, CompatInfo):
            return NotImplemented
        return self.namespace == other.namespace or \
            self.impl == other.impl or \
            self.being == other.being or \
            self.of == other.of

    def __hash__(self) -> int:
        return hash((
                self.namespace,
                self.impl,
                self.being,
                self.of
            ))

    def __repr__(self):
        return f'{{"{self.file}", line {self.line}, {self.decl.value}}}'

class MyJSONifier(json.JSONEncoder):
    def default(self, obj: object):
        if isinstance(obj, CompatInfo):
            return obj.__dict__
        elif isinstance(obj, DeclInfo) or isinstance(obj, BeingInfo):
            return obj.value
        return super().default(obj)

parser = argparse.ArgumentParser(
    prog="CompatCheck", 
    description="Creates a compatibility report for Angelscript exports", 
    formatter_class=argparse.ArgumentDefaultsHelpFormatter
)

parser.add_argument("-i", "--input", help="the directory of files to check")
parser.add_argument("-a", "--against", help="the OpenplanetCore.json file to check against")
parser.add_argument("-o", "--output", help="the output directory of the compatibility reports", default="../docs/")
parser.add_argument("-f", "--filter", help="filter files to check", default="**/*.cpp,**/*.h")
parser.add_argument("-g", "--github", help="store result to github environ", action="store_true")
parser.add_argument("-n", "--no-fail", help="never exit with code other than 0 except when something fatal happens", action="store_true")
parser.add_argument("-p", "--pages", help="prepare documentation for github pages to output directory", action="store_true")
parser.add_argument("-s", "--style", help="the style.css file", default="style.css")
parser.add_argument("-r", "--recursive", action="store_true")

args = parser.parse_args()

print(f"------ CompatCheck: checking against {args.against} ------")

has_unmatched_info = False
matches: dict[DeclInfo, int] = defaultdict(int)
infos: list[CompatInfo] = []
infos_by_being: dict[str, list[CompatInfo]] = defaultdict(list)
def do():
    global has_unmatched_info, matches, infos, infos_by_being

    if not args.github and is_in_github_actions:
        print("skipping post-build event")
        quit(0)

    files: list[str] = []
    for filter in args.filter.split(","):
        files += glob(args.input + "/" + filter, recursive=args.recursive)
        

    for being_info in BeingInfo:
        infos_by_being[being_info.value] = []

    for file in files:
        with open(file, "r") as file:
            try:
                lines = file.readlines()

                tag_count = 0
                for line in lines:
                    if "!CompatMeta" in line:
                        tag_count += 1

                if tag_count % 2 != 0:
                    raise SyntaxError("a tag was opened but not closed")

                seen_tag = False
                for idx, line in enumerate(lines):
                    if "!CompatMeta" in line:
                        seen_tag = not seen_tag
                        if not seen_tag:
                            infos[-1].line = idx + 2
                            infos[-1].file = str(Path(file.name).relative_to(args.input))
                        else:
                            infos.append(CompatInfo())
                            infos[-1].line = idx + 2
                            infos[-1].file = str(Path(file.name).relative_to(args.input))
                        continue
                    elif seen_tag:
                        line = line.strip().removeprefix("//").strip().split(None, 1)
                        command = line[0]
                        value = line[1]

                        infos[-1].From(command, value)
            except Exception as e:
                print(f"could not parse or open {file.name}: {e}")

    infos = list(dict.fromkeys(infos).keys())

    for info in infos:
        infos_by_being[info.being].append(info)

    misses = 0

    class_name_to_idx: dict[str, int] = {}

    with open(args.against, "r") as core:
        core_contents = json.load(core)
        matches_being = 0
        for i, c in enumerate(core_contents["classes"]):
            class_name_to_idx[c["name"]] = i

        for being in infos_by_being:
            is_of_class = being in ["method", "behavior", "prop"]
            being = being if being != "global" else "prop"

            if not is_of_class:
                key = pluralize(being)
                contents = core_contents[key]

                for thing in contents:
                    for info in infos_by_being[being]:
                        if thing["decl" if being not in ["enum", "prop"] else "name"] in info.wanted:
                            matches[info.decl] += 1
                            matches_being += 1

                            info.matched = True
            else:
                for info in infos_by_being[being]:
                    if info.of == "" and is_of_class:
                        raise SyntaxError(f"{info}: {info.being} of what?")
                    
                    contents = core_contents["classes"][class_name_to_idx[info.of]][pluralize(being)]
                    for thing in contents:
                        against = thing["decl"] if being not in ["prop", "behavior"] else ((thing["typedecl"] + " " + thing["name"]) if being != "behavior" else thing["func"]["decl"])
                        if against in info.wanted:
                            matches[info.decl] += 1
                            matches_being += 1

                            info.matched = True

            misses += len(contents) - matches_being
            matches_being = 0

    for being in infos_by_being:
        for info in infos_by_being[being]:
            if not info.matched:
                print(f"unmatched info: {info}")
                has_unmatched_info = True

    complete = (matches[DeclInfo.Full] + matches[DeclInfo.Partial] + matches[DeclInfo.Deleted]) / float(misses) if misses else 0
    non_partial = (matches[DeclInfo.Full] + matches[DeclInfo.Deleted]) / float(misses) if misses else 0

    print(f'{"".join(f"{matches[m]} {m.value} matches, " if matches[m] != 1 else f"{matches[m]} {m.value} match, " for m in matches)}{misses} {"misses" if misses != 1 else "miss"}')
    print(f'{complete:.2%} complete, {non_partial:.2%} non-partial')

    if args.github:
        with open(os.environ["GITHUB_ENV"], "a") as f:
            f.write(f"COMPLETE_PERCENTAGE={complete * 100:.1f}\n")
            f.write(f"NON_PARTIAL_PERCENTAGE={non_partial * 100:.1f}\n")

if args.no_fail:
    try:
        do()        
    except Exception as e:
        print("An exception occured during compatcheck:")
        print(traceback.format_exc())
else:
    do()

if args.pages:
    try:
        os.makedirs(args.output + "/json/")
        with open(args.output + "/json/Output.json", "x") as out:
            pass
        os.remove(args.output + "/json/Output.json")
    except FileExistsError:
        os.remove(args.output + "/json/Output.json")

    with open(args.output + "/json/Output.json", "x") as out:
        json.dump(
            { 
                "export": 
                {
                    "time": str(datetime.datetime.now()),
                    "matches": {key.value: value for key, value in matches.items()}
                }, 
                "compats": infos
            }, 
            out, 
            cls=MyJSONifier, 
            indent=4
        )

    try: os.makedirs(args.output + "/html/")
    except: pass

    index = html(lang="en")(
        head(
            meta(charset="UTF-8"),
            meta(name="viewport", content="width=device-width, initial-scale=1.0"),
            link(rel="stylesheet", href="style.css"),
            title("TwinkieX Compatiblity Report")
        ),
        body(
            h2(
                a(href="https://github.com/TwinkieTweaks/TwinkieX/")("TwinkieX"),
                " Compatibility Report"
            ),
            [info.ToHtml() for info in infos]
        )
    )

    try:
        with open(args.output + "/html/index.html", "x") as out:
            pass
        os.remove(args.output + "/html/index.html")
    except FileExistsError:
        os.remove(args.output + "/html/index.html")

    with open(args.output + "/html/index.html", "x") as out:
        out.write(str(index))

    shutil.copyfile(args.style, args.output + "/html/style.css")

print(f"------ CompatCheck: checked against {args.against} ------")

if not args.no_fail: 
    quit(has_unmatched_info or (len(matches) == 0))