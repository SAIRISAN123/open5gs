def encode(n):
    # Case 1: n = 0
    if n == 0:
        return 0
    
    # Case 2: 1 <= n <= 255
    if 1 <= n <= 255:
        return n * 256
    
    # For n >= 256
    k = n // 256  # Integer division to get k
    m = n % 256   # Modulo to get m
    
    # Case 3: n is a multiple of 256 (m = 0)
    if m == 0:
        return k
    
    # Case 4: n = 256 * k + m where 0 < m < 256
    return m * 256 + k

# Prompt user for input in while loop
# to ensure valid input
while True:
    try:
        user_input = int(input("Enter a non-negative integer: "))
        if user_input < 0:
            print("Please enter a non-negative integer.")
        else:
            result = encode(user_input)
            print(f"Input: {user_input}, Output: {result}")
    except ValueError:
        print("Invalid input. Please enter a non-negative integer.")
