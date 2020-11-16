import re

operations = ['|', '&']


def serialize_and(expression):
    if len(expression.split('&', maxsplit=1)) == 1:
        if expression.startswith('!'):
            expression = f"({expression})"

        return expression

    A, B = expression.rsplit('&', maxsplit=1)

    return f"(&,{serialize_and(A)},{serialize_and(B)})"


def serialize_or(expression):
    if len(expression.split('|', maxsplit=1)) == 1:
        return serialize_and(expression)

    A, B = expression.split('|', maxsplit=1)

    return f"(|,{serialize_or(A)},{serialize_or(B)})"


def serialize(expression):
    if len(expression.split('->', maxsplit=1)) == 1:
        if not any([op in expression for op in operations]):
            if expression.startswith('!'):
                expression = f"({expression})"

            return expression

        return serialize_or(expression)

    A, B = expression.split('->', maxsplit=1)

    return f"(->,{serialize(A)},{serialize(B)})"


def serialize_brakets(expression, replacements):
    for key, value in replacements.items():
        expression = expression.replace(key, f'!{serialize_or(value[2:-1])}')

    return expression


def prepare_expression(expression):
    all_brakets = re.findall(r'!\(\S*\)', expression)
    replacements = dict([(f'_{i+1}', all_brakets[i])
                         for i in range(len(all_brakets))])

    for key, value in replacements.items():
        expression = expression.replace(value, key)

    return expression, replacements


def solve(string):
    expression, replacements = prepare_expression(a)
    serialized_expression = serialize(expression)
    total_expression = serialize_brakets(serialized_expression, replacements)

    return total_expression


if __name__ == '__main__':
    a = input()

    print(solve(a))
