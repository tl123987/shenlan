import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import interp1d
from scipy.interpolate import CubicSpline

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

# 转为 NumPy 数组
x_data = np.array(x_data)
y_data = np.array(y_data)

# 定义插值函数（线性插值）
interp_func = interp1d(x_data, y_data, kind='linear')
#cs = CubicSpline(x_data, y_data)
# 生成插值后的 x 数据（范围从最小到最大值，步长为 0.1）
x_interp = np.arange(x_data.min(), x_data.max(), 0.5)
y_interp = interp_func(x_interp)

# 绘制原始数据和插值数据
plt.figure(figsize=(10, 6))
plt.plot(x_data, y_data, 'o', label='Original Data', color='red')  # 原始数据
plt.plot(x_interp, y_interp, '-', label='Interpolated Data', color='blue')  # 插值后的数据
plt.title('Data Interpolation')
plt.xlabel('X-axis')
plt.ylabel('Y-axis')
plt.legend()
plt.grid(True)
plt.show()
