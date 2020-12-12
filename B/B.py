from A import solve


def right_expression(expression):
    stack = []

    for s in expression:
        if len(stack) == 0 and s == ')':
            return False
        if s == '(':
            stack.append(s)
        elif s == ')' and stack[-1] == '(':
            stack.pop()


    return len(stack) == 0


def find_second_comma(expression):
    first_comma = expression.index(',')

    for i in range(first_comma+1, len(expression)):
        if right_expression(expression[first_comma:i+1]):
            return i


def build_expression(s_expression):
    if not ',' in s_expression:
        return s_expression

    s_expression = s_expression[1:-1]

    first_comma = s_expression.index(',')
    second_comma = find_second_comma(s_expression)
    op, A, B = s_expression[:first_comma], s_expression[first_comma+1:second_comma+1], s_expression[second_comma+2:]

    if not '(' in s_expression:
        return f'({A}{op}{B})'

    return f'({build_expression(A)}{op}{build_expression(B)})'

a = input()

hypothesis, result = a.replace(' ', '').split('|-')

axioms = {
    '1': 'α->β->α',
    '2': '(α->β)->(α->β->γ)->(α->γ)',
    '3': 'α->β->α&β',
    '4': 'α&β->α',
    '5': 'α&β->β',
    '6': 'α->α|β',
    '7': 'β->α|β',
    '8': '(α->γ)->(β->γ)->(α|β->γ)',
    '9': '(α->β)->(α->!β)->!α',
    '10': '!!α->α',
}

r = []; x = []

a = input().replace(' ', '')

while a != result:
    r.append(build_expression(solve(a)))
    x.append(solve(a))
    a = input().replace(' ', '')
