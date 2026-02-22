dictionary = [
    (3, "three1"),
    (14, "fourteen1"),
    (15, "fifteen1"),
    (9, "nine1"),
    (15, "fifteen2"),
    (2, "two1"),
    (6, "six1"),
    (5, "five"),
    (32, "thirtytwo1"),
]

d = []
while s := input():
    a, b = s.split("\t")
    d.append([int(a), b])


def prefix_sum(arrow):
    for i in range(len(arrow) - 1):
        arrow[i + 1] = arrow[i] + arrow[i + 1]
    return arrow


def counting_sort(arrow: list) -> list:
    max_element = max(x[0] for x in arrow)

    counting_arrow = [0 for i in range(max_element + 1)]
    for element in arrow:
        counting_arrow[element[0]] += 1
    counting_arrow = prefix_sum(counting_arrow)

    sorted_arrow = [[0, ""] for i in range(len(arrow))]

    for element in arrow[::-1]:
        sorted_arrow[counting_arrow[element[0]] - 1] = element
        counting_arrow[element[0]] -= 1

    return sorted_arrow


print(*counting_sort(d), sep="\n")
