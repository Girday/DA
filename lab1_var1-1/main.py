s = [3, 14, 15, 9, 15, 2, 6, 5, 32]


def prefix_sum(arrow):
    for i in range(len(arrow) - 1):
        arrow[i + 1] = arrow[i] + arrow[i + 1]
    return arrow


def counting_sort(arrow):
    max_element = max(arrow)

    counting_arrow = [0 for i in range(max_element + 1)]
    for element in arrow:
        counting_arrow[element] += 1
    counting_arrow = prefix_sum(counting_arrow)

    sorted_arrow = [0 for i in range(len(arrow))]

    for element in arrow[::-1]:
        sorted_arrow[counting_arrow[element] - 1] = element
        counting_arrow[element] -= 1

    return sorted_arrow


print(s)
print(counting_sort(s))
