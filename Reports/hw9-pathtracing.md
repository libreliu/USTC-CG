# 作业 9 PathTracing

刘紫檀 PB17000232

## 效果展示

目前只实现了重要性采样之前的部分。由于肝不动了而没继续做（靠，困到恍惚）

256 spp，400x225

![rst](assets/rst.png)

256spp，1024x576![rst](assets/rst-1588453575034.png)

上面这张图在 i5-3210M (2C4T, 撞功耗之后 2.1GHz) 大概要渲染 6min。

可以看到

- 顶灯的法线照明区域判断
- 间接光
- 直接光的阴影
- 环境光

都有了。

## 调试心得

下面记录一下我犯过的蠢：

1. ![rst_fancy](assets/rst_fancy.png)

   因为在给间接光计算辐射度的时候，除以了随机变量 `rand01` 本身，而不是其概率密度，强行引入噪点

   ![rst_fancy_2](assets/rst_fancy_2.png)

   并且噪点会随着 spp 增多而加重；同时，在处理间接光的时候，忘记设置 `last_bounce_specular` ，从而让 `area_light` 和 `env_light` 反复参与计算..然后就会变得“贼亮”（如上图）

2. 搞不明白正反，还用错了发现，用成了 intersection 的法线（判断顶灯辐射方向时）

   ![rst_fancy_3](assets/rst_fancy_3.png)

   最后还是先让 `cos_theta` 变小一点，把灯往下移一点，从而限制一下灯的“FOV”，来看明白情况。

## 原理补充

Path Tracing 是一种 RayTracing 实现方法。

其它的感觉助教的已经很详细了。

### 重要性采样

设有连续函数 $ g(x) $，现在需要知道其积分 $ \int_a^b g(x) dx $。

设 $ g(x) = f(x) p(x) $ ，其中 $ p (x) $ 满足 $  \int_a^b p(x) dx = 1 $（显然这样的 $ f(x) $ 和 $ p(x) $ 存在），则

$$
\int_a^b g(x) dx =  \int_a^b f(x) p(x) dx = E_{p(x)}[f(x)] \approx \hat{E}_{p(x)}[f(x)] = \frac{1}{N}\Sigma_{i=1}^N\frac{g(x_i)}{p(x_i)} \tag {Using CLE}
$$
不妨设随机变量 $ X $ 满足概率密度为 $ p(x) $ 定义的分布，则 $ g(x) $ 为随机变量 $ Y = f(X) $ 的概率密度分布。

同时，**此期望**的样本方差：
$$
\begin{align}
Var(\hat{E}(X)) = \frac{1}{N^2} \sum_{i=1}^{N} Var(\frac{g(x_i)}{p(x_i)}) = \frac{1}{N} Var(\frac{g(x)}{p(x)})
\end{align}
$$

则如果 $ g(x) \rightarrow p(x) $，方差就会更小，而根据 CLE， $ \hat{E} \sim N(E, Var(\hat{E})) (n \rightarrow \infty) $ 结果也会更精确些。

