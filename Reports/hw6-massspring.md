# 作业 6 弹簧质点系统

刘紫檀 PB17000232

## 原理

### 欧拉隐式方法

$$
\boldsymbol x_{n+1}=\boldsymbol x_n+h\boldsymbol v_{n+1},\\
\boldsymbol v_{n+1}=\boldsymbol v_n+h\boldsymbol M^{-1}(\boldsymbol f_{int}(\boldsymbol x_{n+1}) +\boldsymbol f_{ext})
$$

记
$$
\boldsymbol y =\boldsymbol x_n + h\boldsymbol v_n + h^2\boldsymbol M^{-1}\boldsymbol f_{ext}, \tag{*}
$$
则原问题转化为求解关于$\boldsymbol x$的方程：
$$
\boldsymbol g(\boldsymbol x) = \boldsymbol M(\boldsymbol x-\boldsymbol y) -h^2\boldsymbol f_{int}(\boldsymbol x) = 0, \tag{1}
$$
利用牛顿法求解该方程，主要迭代步骤：
$$
\boldsymbol x^{(k+1)}=\boldsymbol x^{(k)}-(\nabla \boldsymbol g(\boldsymbol x^{(k)}))^{-1}\boldsymbol g(\boldsymbol x^{(k)}).
$$

迭代初值可选为$\boldsymbol x^{(0)}=y$ .

迭代得到位移$x$后更新速度$v_{n+1}=(x_{n+1}-x_{n})/h$

其中 $ \boldsymbol f_{int}(x) $ 如下求得：
$$
\boldsymbol x_1所受弹力：     \boldsymbol f_1(\boldsymbol x_1,\boldsymbol x_2)=k(||\boldsymbol x_1-\boldsymbol x_2||-l)\frac{\boldsymbol x_2-\boldsymbol x_1}{||\boldsymbol x_1-\boldsymbol x_2||}
$$
其中 $ \nabla \boldsymbol g( \boldsymbol x^{(k)}) $ 应如下求得：
$$
\begin{align}
\nabla \boldsymbol g( \boldsymbol x^{(k)}) = 

\begin{pmatrix} \frac{\part \boldsymbol g_{x_1}}{\part \boldsymbol x} & \frac{\part \boldsymbol g_{x_2}}{\part \boldsymbol x} & \frac{\part \boldsymbol g_{x_3}}{\part \boldsymbol x} \end{pmatrix}

&= \bold M (\nabla \boldsymbol x - \nabla \boldsymbol y) - h^2 \nabla \boldsymbol f_{int}(\boldsymbol x) \\

&= \bold M \cdot (\bold I - \bold O) - h^2 \nabla \boldsymbol f_{int}( \boldsymbol x)

\end{align}
$$
下面求 $ \nabla \boldsymbol f_{int}( \boldsymbol x) $ 。考虑此质点 $ i $ 周围的弹簧 $ j \in N(i) $，得到如下关系：（此处 $ x_{(i)} $ 表示第 i 个点的方程）
$$
\begin{align}
\nabla f_{int}(x_{(i)}) &= \sum_{j \in N(i)} \nabla (k(||\boldsymbol x_{(i)}-\boldsymbol x_{(j)}||-l)\frac{\boldsymbol x_{(j)}-\boldsymbol x_{(i)}}{||\boldsymbol x_{(i)}-\boldsymbol x_{(j)}||}) \\

&=  \sum_{j \in N(i)} k(\frac{l}{||\boldsymbol x_{(i)}-\boldsymbol x_{(j)}||}-1)\boldsymbol I-kl||\boldsymbol x_{(i)}-\boldsymbol x_{(j)}||^{-3}(\boldsymbol x_{(i)}-\boldsymbol x_{(j)}) (\boldsymbol x_{(i)}-\boldsymbol x_{(j)})^T

\end{align}
$$
故原式如下：
$$
\begin{align}
\nabla \boldsymbol g( \boldsymbol x^{(k)}_{(i)} ) &= 

\bold M \cdot (\bold I - \bold O) - h^2 \nabla \boldsymbol f_{int}( \boldsymbol x_{(i)}^{(k)}) \\

&= \bold M - h^2\sum_{j \in N(i)} [ k(\frac{l}{||\boldsymbol x_{(i)}^{(k)}-\boldsymbol x_{(j)}^{(k)}||}-1)\boldsymbol I-kl||\boldsymbol x_{(i)}^{(k)}-\boldsymbol x_{(j)}^{(k)}||^{-3}(\boldsymbol x_{(i)}^{(k)}-\boldsymbol x_{(j)}^{(k)}) (\boldsymbol x_{(i)}^{(k)}-\boldsymbol x_{(j)}^{(k)})^T]


\end{align}
$$
牛顿法迭代公式可以写为如下形式：
$$
\boldsymbol x^{(k+1)}_{(i)} = \boldsymbol x^{(k)}_{(i)} - (\nabla \boldsymbol g(\boldsymbol x^{(k)}_{(i)}))^{-1}\boldsymbol g(\boldsymbol x^{(k)}_{(i)}) \\

(\nabla \boldsymbol g(\boldsymbol x^{(k)}_{(i)})) \boldsymbol x^{(k+1)}_{(i)}  =  - \boldsymbol g(\boldsymbol x^{(k)}_{(i)}) + (\nabla \boldsymbol g(\boldsymbol x^{(k)}_{(i)}))\boldsymbol x^{(k)}_{(i)}
$$
其中每个 $ \boldsymbol x_{(i)} $ 都是三个参数的一个矢量，所以我们可以为每一个点列一个三元的方程组。

但是，由于弹力的对称性，这样列方程会浪费很多计算资源。我们可以将所有点的方程拼在一起，组装为一个大稀疏矩阵，一下求出全部的 $ \boldsymbol x^{(k+1)} $ 。

拼装后

$$
\boldsymbol x ^ {(k+1)} = [\boldsymbol x^{(k+1)}_{(0)}, \dots , \boldsymbol x^{(k+1)}_{(n)} ] ^ \bold T =  [(\boldsymbol x^{(k+1)}_{(0)})_0, \dots , (\boldsymbol x^{(k+1)}_{(n)})_2 ] ^ \bold T \\

\boldsymbol g(\boldsymbol x^{(k)}) = [\boldsymbol g(\boldsymbol x^{(k)}_{(0)}), \dots, \boldsymbol g(\boldsymbol x^{(k)}_{(n)})] ^ \bold T = [(\boldsymbol g(\boldsymbol x^{(k)}_{(0)}))_0, \dots, (\boldsymbol g(\boldsymbol x^{(k)}_{(n)}))_2] ^ \bold T \\

$$
迭代到 $ (1) $ 式的误差足够小后，更新所有位移和速度。

#### 考虑位移约束的情况

仅考虑真正的自由坐标，降低问题的维数，具体如下：

将所有 n 个质点的坐标列为列向量 $x\in R^{3n}$，将所有 m 个自由质点坐标（无约束坐标）列为列向量 $x_f\in R^{3m}$,则两者关系：
$$
\boldsymbol x_f=\boldsymbol K\boldsymbol x,\\  \boldsymbol x=\boldsymbol K^T\boldsymbol x_f+\boldsymbol b,
$$
其中 $K\in R^{3m\times 3n}$ 为单位阵删去约束坐标序号对应行所得的稀疏矩阵，$b$ 为与约束位移有关的向量，计算为 $b=x-K^TKx$, 若约束为固定质点则 $b$ 为常量。由此我们将原本的关于 $x$ 的优化问题转化为对 $x_f$ 的优化问题：欧拉隐式方法中求解方程为：
$$
\boldsymbol g_1(\boldsymbol x_f) = K(\boldsymbol M(\boldsymbol x-\boldsymbol y) -h^2\boldsymbol f_{int}(\boldsymbol x)) = 0,\\
梯度：\nabla_{x_f} \boldsymbol g_1(\boldsymbol x_f) = K\nabla_{x} \boldsymbol g(\boldsymbol x)K^T,\\
$$
给原来的 $ \nabla x g(x) $ 乘上 K 和 K 转置就行了。

> 限于时间没有做位移约束的情况，只搭了框架..

## 实现

- 给定“边界”在里面到处乱撞的模式

  > 好，限于时间这个也没有做。。

- ~~还有没有边界到处乱飞的模式~~

### 效果

参考录屏即可。