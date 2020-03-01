# ImageWraper 实验报告

刘紫檀 PB17000232

## 原理

### IDW

Inverse distance-weighted 插值方法通过计算下式进行插值：
$$
\pmb{f}(\pmb{p})=\sum w_i(\pmb{p})\pmb{f}_i(\pmb{p})
$$
$\pmb{f}_i(\pmb{p})$ 是局域项，其中

$$
w_i(\pmb{p})=\frac{\sigma_i(\pmb{p})}{\sum \sigma_j(\pmb{p})}
$$

$$
\sigma_i(\pmb{p})=\frac{1}{\|\pmb{p}-\pmb{p_i}\|^u}
$$

论文中使用了一次的局域项
$$
\pmb f_i( \pmb p)= \pmb q_i+ \pmb T_i(  \pmb p- \pmb p_i)
$$
并且优化目标是让这个式子
$$
E_i(\mathbf{T}) = \sum^n_{j=1,j \ne i} \mathbf{\sigma_i} (\mathbf{p}_j) \left\| \mathbf{q}_i+ \begin{pmatrix} t_{11} & t_{12} \\ t_{21} & t_{22}  \end{pmatrix} (\mathbf{p}_j - \mathbf{p}_i) - \mathbf{q}_j \right\|^2
$$
最小。此时我们可以让 $ E_i(\bf{T}) $ 关于 $ \bf T $  求导，首先整理 $ E_i(\mathbf{T}) $：
$$
E_i(\mathbf{T}) = \sum^n_{j=1,j \ne i} \mathbf{\sigma_i} (\mathbf{p}_j) ( \mathbf{q}_i+ \mathbf{T} (\mathbf{p}_j - \mathbf{p}_i)-\mathbf{q}_j ) ^\top ( \mathbf{q}_i+ \mathbf{T} (\mathbf{p}_j - \mathbf{p}_i) - \mathbf{q}_j )
$$
求导
$$
\begin{align}
\frac{\mathrm{d}E_i}{\mathrm{d}\mathbf{T}} =& \sum^n_{j=1,j \ne i} \frac{\mathrm{d} (\mathbf{\sigma_i} (\mathbf{p}_j) ( \mathbf{q}_i+ \mathbf{T} (\mathbf{p}_j - \mathbf{p}_i)-\mathbf{q}_j ) ^\top ( \mathbf{q}_i+ \mathbf{T} (\mathbf{p}_j - \mathbf{p}_i) - \mathbf{q}_j ))} {\mathrm{d}\mathbf{T}} \\


=& \sum^n_{j=1,j \ne i} \mathbf{\sigma_i} (\mathbf{p}_j) \cdot 

[ \frac{ \mathrm{d}( \mathbf{q}_i+ \mathbf{T} (\mathbf{p}_j - \mathbf{p}_i)-\mathbf{q}_j )^\top}{\mathrm{d}\mathbf{T}}  ( \mathbf{q}_i+ \mathbf{T} (\mathbf{p}_j - \mathbf{p}_i) - \mathbf{q}_j )) \\ 

+ & ( \mathbf{q}_i+ \mathbf{T} (\mathbf{p}_j - \mathbf{p}_i) - \mathbf{q}_j )^\top \frac{\mathrm{d}( \mathbf{q}_i+ \mathbf{T} (\mathbf{p}_j - \mathbf{p}_i)-\mathbf{q}_j )}{\mathrm{d}\mathbf{T}}   ] \\

=& \sum^n_{j=1,j \ne i} \mathbf{\sigma_i} (\mathbf{p}_j) \cdot 

[ (\mathbf p_j- \mathbf p_i)^\top  ( \mathbf{q}_i+ \mathbf{T} (\mathbf{p}_j - \mathbf{p}_i) - \mathbf{q}_j )) \\ 

+ & (\mathbf{T} (\mathbf{p}_j - \mathbf{p}_i) - (\mathbf{q}_j -  \mathbf{q}_i) )^\top  (\mathbf p_j- \mathbf p_i) ] \\

=& \sum^n_{j=1,j \ne i} \mathbf{\sigma_i} (\mathbf{p}_j) \cdot 

[ (\mathbf p_j- \mathbf p_i) ^\top \mathbf{T} (\mathbf{p}_j - \mathbf{p}_i)  - (\mathbf p_j- \mathbf p_i)^\top (\mathbf{q}_j - \mathbf{q}_i) \\ 

+ & (\mathbf{p}_j - \mathbf{p}_i)^\top \mathbf{T}^\top  (\mathbf p_j- \mathbf p_i) - (\mathbf{q}_j -  \mathbf{q}_i) ^\top (\mathbf p_j- \mathbf p_i) ] 

=0

\end{align}
$$
则得到约束
$$
\sum^n_{j=1,j \ne i} \mathbf{\sigma_i} (\mathbf{p}_j) \cdot 

[ (\mathbf{p}_j - \mathbf{p}_i)^\top \mathbf{T} (\mathbf p_j- \mathbf p_i)  +  (\mathbf{p}_j - \mathbf{p}_i)^\top \mathbf{T}^\top  (\mathbf p_j- \mathbf p_i) ] \\

= \sum^n_{j=1,j \ne i}  \mathbf{\sigma_i} (\mathbf{p}_j) \cdot  [(\mathbf p_j- \mathbf p_i)^\top (\mathbf{q}_j - \mathbf{q}_i) + (\mathbf{q}_j -  \mathbf{q}_i) ^\top (\mathbf p_j- \mathbf p_i)]
$$

化简得到
$$
\sum^n_{j=1,j \ne i} \mathbf{\sigma_i} (\mathbf{p}_j) \cdot 

 (\mathbf{p}_j - \mathbf{p}_i)^\top (\mathbf{T}^\top  +  \mathbf{T}) (\mathbf p_j- \mathbf p_i)      \\

= \sum^n_{j=1,j \ne i}  \mathbf{\sigma_i} (\mathbf{p}_j) \cdot  [(\mathbf p_j- \mathbf p_i)^\top (\mathbf{q}_j - \mathbf{q}_i) + (\mathbf{q}_j -  \mathbf{q}_i) ^\top (\mathbf p_j- \mathbf p_i)]
$$
同时
$$
\frac{\mathrm{d}^2E_i}{\mathrm{d}\mathbf{T}^2}=2 (\sum^n_{j=1,j \ne i} \mathbf{\sigma_i} (\mathbf{p}_j) \cdot \|\mathbf p_j- \mathbf p_i\|^2)  \mathbf{I} \ge 0
$$
则满足上式的解均为极小值点；代入得到



下面推导其满足的线性方程组，设
$$

$$




### RBF




