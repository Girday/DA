dictionary = [
    (3, "three1"),
    (2, "two1"),
    (14, "fourteen1"),
    (15, "fifteen1"),
    (9, "nine1"),
    (15, "fifteen2"),
    (2, "two2"),
    (6, "six1"),
    (5, "five1"),
    (32, "thirtytwo1"),
]

d = []
while s := input():
    a, b = s.split("\t")
    d.append([int(a), b])


def prefix_sum(array):
    for i in range(len(array) - 1):
        array[i + 1] = array[i] + array[i + 1]
    return array


def counting_sort(array: list) -> list:
    max_element = max(x[0] for x in array)

    counting_array = [0 for i in range(max_element + 1)]
    for element in array:
        counting_array[element[0]] += 1
    counting_array = prefix_sum(counting_array)

    sorted_array = [[0, ""] for i in range(len(array))]

    for element in array[::-1]:
        sorted_array[counting_array[element[0]] - 1] = element
        counting_array[element[0]] -= 1

    return sorted_array


print(*counting_sort(dictionary), sep="\n")
