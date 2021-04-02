from ast import literal_eval
import re

REGEX = re.compile(
    r'\s*(?:(?P<key>[a-zA-Z0-9_]+)\s*\=)?\s*(?P<value>".*"|[^,]+)\s*(?:,|$)'
)


def eval_literal(string):
    try:
        value = literal_eval(string)
    except Exception:
        value = string
    return value


def string_to_func_inputs(text):
    args = []
    kwargs = {}
    for key, value in REGEX.findall(text):
        if key:
            kwargs[key.strip()] = eval_literal(value.strip())
        else:
            args.append(eval_literal(value.strip()))
    return args, kwargs
