import matplotlib.pyplot as plt

# 读取数据
filename = "src/stanley_control/data/1.txt"  # 替换为你的文件名
x_data = []
y_data = []

with open(filename, 'r') as file:
    for line in file:
        if line.strip():  # 确保不是空行
            x, y = map(float, line.split(','))  # 使用逗号分隔数据
            x_data.append(x)
            y_data.append(y)

# 绘制图形
plt.figure(figsize=(8, 6))
plt.plot(x_data, y_data, marker='o', linestyle='-', color='b', label='Data Line')
plt.title('Line Plot from TXT Data (Comma Separated)')
plt.xlabel('X-axis')
plt.ylabel('Y-axis')
plt.legend()
plt.grid(True)
plt.show()
