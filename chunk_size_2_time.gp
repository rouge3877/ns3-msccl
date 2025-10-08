# multi_file_plot.gp

# --- TERMINAL AND OUTPUT ---
# 定义输出文件格式和路径
# output_file 变量需要从外部传入
set terminal pdfcairo monochrome rounded noenhanced font "Times-New-Roman,12" size 5,3.5
set output output_file

# --- DATA SOURCE ---
# 定义数据文件的列分隔符
set datafile separator ","

# --- STYLE DEFINITIONS ---
# 定义多种线条和点的样式，以便区分不同的输入文件
# 您可以根据需要添加更多样式 (set style line 3, 4, 5...)
set style line 1 lc "black" lt 1 lw 2.5 pt 7 ps 1.0 # 实线, 圆点
set style line 2 lc "black" lt 2 lw 2.5 pt 9 ps 1.0 # 虚线, 三角
set style line 3 lc "black" lt 4 lw 2.5 pt 5 ps 1.0 # 点划线, 方块
set style line 4 lc "black" lt 5 lw 2.5 pt 11 ps 1.0 # 长划线, 菱形

# --- HELPER FUNCTION ---
# 定义一个函数，用于从文件名中提取前缀 (例如 "data/file1.csv" -> "file1")
# It finds the part between the last '/' and the last '.'
get_prefix(s) = ( \
    end_of_path = strstrt(s, '/') > 0 ? strstrt(s, '/') : 0, \
    start_of_ext = strstrt(s, '.'), \
    start_of_ext > end_of_path ? s[end_of_path+1:start_of_ext-1] : s[end_of_path+1:strlen(s)] \
)

# --- TITLE ---
set title "MSCCL Performance vs. Chunk Size" font "Times-New-Roman,16"

# --- MARGINS ---
set lmargin at screen 0.15
set rmargin at screen 0.95
set bmargin at screen 0.15
set tmargin at screen 0.90

# --- AXES AND GRID ---
set xlabel "Chunk Size (KB)" font "Times-New-Roman,14"
set ylabel "Time Taken (ns)" font "Times-New-Roman,14"
set border 3 front linetype -1 linewidth 1.0
set tics out nomirror
set grid back linetype 0 linecolor "gray80"

# --- AXIS SCALES & TICS ---
set logscale x 2
unset mxtics
set mytics 2

# --- DYNAMIC PLOT HANDLING (REMOVED) ---
# 原始脚本中用于处理单个数据点的 'stats' 和 'if' 逻辑已被移除。
# 在绘制多条曲线时，Gnuplot 的自动 X 轴范围功能通常更为健壮。

# --- LEGEND (KEY) ---
set key top left nobox samplen 2

# --- PLOT COMMAND ---
# 使用 'plot for' 循环来绘制 input_files 变量中列出的所有文件
# 1. 'words(input_files)' 获取文件总数
# 2. 'word(input_files, i)' 获取第 i 个文件名
# 3. 'ls i' 将 style line 1, 2, 3... 依次应用到每个文件
# 4. 'title get_prefix(...)' 调用辅助函数，使用文件前缀作为图例标题
plot for [i=1:words(input_files)] word(input_files, i) using 1:2:xtic(1) with linespoints ls i title get_prefix(word(input_files, i))