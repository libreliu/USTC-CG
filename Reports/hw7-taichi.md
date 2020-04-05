# 作业 7 Taichi 模拟系统

刘紫檀 PB17000232

## 探究

### particle 分布
首先，我们可以考虑圆内的均匀分布：

$$
\left\{
    \begin{eqnarray}
        x = r^2 \cos \theta \\
        y = r^2 \sin \theta 
    \end{eqnarray}
\right.
$$

其中 $ r $ 和 $ \theta $ 均为均匀分布。

> 证明这样产生的分布是均匀的也非常简单，只要考虑其 $ g: (r, \theta) \rightarrow (x,y) $ 变换的雅可比行列式即可。
> 此行列式计算可得为 2，则由于 $ (r, \theta) $ 的概率密度是常数，可知 $ (x,y) $ 的概率密度也为常数。

我们只需要修改 particle 初始化的代码即可实现我们的更改，修改和对应的效果如下：

### sample 数量对仿真结果的影响

### 论文中水轮的模拟

### 拼接图像

在高精度的模拟中，渲染速度太慢（一帧不卡，两帧流畅.jpg），以至于实时不起来，这时候就要用 `canvas.img.write_as_image(fmt::format("/tmp/screenrec/{:05d}.png", frame++));` 这个操作，同时记得提前创建好对应的文件夹。

> 考虑写文件的时间，我的「雪泥」的帧数在 0.1～1FPS 上下。

录制完毕后，用 `ffmpeg -r 30 -pattern_type glob -i '*.png' -c:v libx264 out.mp4` 来生成视频文件。

## MLS-MPM 方法分析

### Intro

之前用到的数值 PDE 方法主要集中在对点的直接差分求解上，而有限元方法是将其转化为能量，并且求解其变分问题的解。

### Weighted Residual Method

设 L 是微分算子，则

$$
L(f(x)) = g(x)
$$

设解有如下形式

$$
f(x) = \sum_{i=1}^{n} a_i \psi_i (x)
$$

这些 $ \psi_i(x) $ 被称为 trial function，而

$$
R = g(x) - L[\sum_{i=1}^{n} a_i \psi_i (x)]
$$

被定义为残差。有三种基本方法来优化这个残差：
- Galerkin 方法：$ \intg_D f_i R \mathrm{d} D = 0,\ i = 1, ..., n $
- 最小二乘法：$ \frac{\partial}{\partial a_i} \intg_D R^2 dD = 0,\ i = 1, ..., n $

注意到这里的 trial function 必须满足所有的边界条件，在此基础上优化才能得到一个比较好的解。

### Ritz analysis

Ritz 方法中，我们把 trial function 替换成能量方法中的能量，并用能量的最小化条件生成 n 个偏导数 = 0 的方程。

此方法具有如下特点：
- Ritz 的 trial funciton 只需要满足 essential boundary conditions
  > 自然边界条件（就是 Neumann b.c.）指对容许函数在固定边界上的值不加限制的情形下，极值函数由于使得一阶变分为零而在边界上必须满足的条件。
- $ \delta \Sigma = 0 $ 会优化 internal equilibrium 和 natural b.c. 的偏差
- 系数矩阵是对称阵

### Constitutive Relations

- 弹性性质
  - $ \sigma_{ij} = f(\epsilon_{ij}) $，其中 $ \sigma $ 是应力， $ \epsilon $ 是应变