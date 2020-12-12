operations = ['|', '&']


def serialize_not(expression):
    i = 0
    while expression[i] == '!':
        i += 1

    return f"(!" * i + expression[i:] + ')' * i


def serialize_and(expression):
    if len(expression.split('&', maxsplit=1)) == 1:
        return serialize_not(expression)

    A, B = expression.rsplit('&', maxsplit=1)

    return f"(&,{serialize_and(A)},{serialize_and(B)})"


def serialize_or(expression):
    if len(expression.split('|', maxsplit=1)) == 1:
        return serialize_and(expression)

    A, B = expression.rsplit('|', maxsplit=1)

    return f"(|,{serialize_or(A)},{serialize_or(B)})"


def serialize(expression):
    if len(expression.split('->', maxsplit=1)) == 1:
        if not any([op in expression for op in operations]):
            return serialize_not(expression)

        return serialize_or(expression)

    A, B = expression.split('->', maxsplit=1)

    return f"(->,{serialize(A)},{serialize(B)})"


def serialize_brakets(expression, replacement):
    keys = sorted(replacement.keys(), key=lambda x: int(x[1:-1]), reverse=True)

    for key in keys:
        value = replacement[key]
        if value.startswith('!(') and value.endswith(')'):
            new_value = f'(!{serialize(value[2:-1])})'
        elif value.startswith('(') and value.endswith(')'):
            new_value = f'{serialize(value[1:-1])}'
        else:
            new_value = serialize(value)

        expression = expression.replace(key, new_value)

    for key in keys:
        if key in expression:
            expression = serialize_brakets(expression, replacement)

    return expression


def get_all_brakets(expression):
    all_brakets = []

    symbols = []
    indexes = []

    i = 0
    while i <= len(expression) - 1:
        if expression[i] == '(':
            indexes.append(i)
            symbols.append('(')
        elif expression[i:i+2] == '!(':
            symbols.append('!(')
            indexes.append(i)
            i += 2; continue
        elif expression[i] == ')' and symbols[-1] == '(':
            all_brakets.append(expression[indexes[-1]:i+1])
            indexes.pop()
            symbols.pop()
        elif expression[i] == ')' and symbols[-1] == '!(':
            all_brakets.append(expression[indexes[-1]:i+1])
            indexes.pop()
            symbols.pop()

        i += 1

    return all_brakets


def prepare_expression(expression):
    all_brakets = get_all_brakets(expression)
    all_brakets.sort(key=lambda x: len(x))
    replacement = {}

    for i in range(len(all_brakets)):
        value, key = all_brakets[i], f"[{i+1}]"
        replacement[key] = value

        expression = expression.replace(value, key)

        for j in range(len(all_brakets)):
            all_brakets[j] = all_brakets[j].replace(value, key)

    return expression, replacement


def solve(string):
    expression, replacement = prepare_expression(a)
    serialized_expression = serialize(expression)
    total_expression = serialize_brakets(serialized_expression, replacement)

    return total_expression


if __name__ == '__main__':
    a = input()

    print(solve(a))
