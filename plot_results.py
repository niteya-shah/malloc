import numpy as np
import mpl_scatter_density
import matplotlib.pyplot as plt
import pandas as pd
import cmasher as cmr
from matplotlib import colorbar
import matplotlib.colors as colors
from matplotlib.ticker import FuncFormatter
from matplotlib.ticker import LogFormatter
import struct

kilobyte = 1024.0
megabyte = kilobyte * 1024.0
gigabyte = megabyte * 1024.0
malloc_max = 100 * megabyte

cmap = cmr.get_sub_cmap('nipy_spectral', 0.03, 0.96)
cbar_ticks = [1, 16, 256, 2 * kilobyte, kilobyte*10, megabyte, megabyte*10, malloc_max]

def format_bytes(bytes):
    if bytes < 1024:
        return f"{bytes:.0f} B"
    elif bytes == 1024:
        return "1 kilobyte"
    elif bytes < 1024 * 1024:
        return f"{bytes/1024:.0f} KB"
    elif bytes == 1024 * 1024:
        return "1 MB"
    elif bytes < 1024 * 1024 * 1024:
        return f"{bytes/1024/1024} MB"
    else:
        return f"{1024.0/1024.0/1024.0} GB"

def get_marker(type):
    if type == 'malloc':
        return 'o'
    elif type == 'calloc':
        return 'x'
    else:
        return '*'

class ColorbarFormatter(LogFormatter):
    def __call__(self, x, pos = None):
        return format_bytes(x)

name = 'data_tf'
path = '/home/touchdown/work/c++/malloc/' + name
dt = np.dtype([('alloc_size',np.intp),('start_sec',np.intp),
                  ('start_nsec',np.intp),('stop_sec',np.intp),
                  ('stop_nsec',np.intp)])


df_malloc = pd.DataFrame(np.fromfile(path + '/malloc_data.txt', dtype=dt))
df_malloc['Type'] = 'malloc'
df_calloc = pd.DataFrame(np.fromfile(path + '/calloc_data.txt', dtype=dt))
df_calloc['Type'] = 'calloc'
df_realloc = pd.DataFrame(np.fromfile(path + '/realloc_data.txt', dtype=dt))
df_realloc['Type'] = 'realloc'
df_free = pd.DataFrame(np.fromfile(path + '/free_data.txt', dtype=dt))
df_free['Type'] = 'free'
# del df_free

for df in [df_malloc, df_calloc, df_realloc, df_free]:
# for df in [df_free]:
    df["alloc_time"] = (df.stop_sec - df.start_sec) * 1e3 + (df.stop_nsec - df.start_nsec) * 1e-6
    if len(df) > 0:
        df["start"] = (df.start_sec - df.loc[0].start_sec) * 1e3 + (df.start_nsec - df.loc[0].start_nsec) * 1e-6

df_alloc = pd.concat([df_malloc, df_calloc, df_realloc])
df1 = df_alloc[["Type", "start", "alloc_size", "alloc_time"]]
del df_alloc, df_malloc, df_calloc, df_realloc

total_time  = 18.32
df_free.alloc_time.sum()/total_time/10
df1.alloc_time.sum()/total_time/10
df_free[df_free.alloc_time > 1].alloc_time.sum()/total_time/10
df1[df1.alloc_time > 0.5].alloc_time.sum()/total_time/10

## %%
fig, ax = plt.subplots(2, sharex=True)
for i in df1["Type"].unique():
    df = df1[df1["Type"] == i]
    density = ax[0].scatter(df.start, df.alloc_time, cmap=cmap,  c=df.alloc_size, s=3, marker=get_marker(i), norm=colors.LogNorm(vmin=cbar_ticks[0],vmax=cbar_ticks[-1]))
density = ax[1].scatter(df_free.start, df_free.alloc_time, cmap=cmap,  c=df_free.alloc_size, s=3, norm=colors.LogNorm(vmin=cbar_ticks[0],vmax=cbar_ticks[-1]))
for i in range(2):
    ax[i].set_yscale("log")
    ax[i].set_facecolor('#E0E0E0')
    ax[i].set_xlabel('Time(ms)')
    ax[i].set_ylabel('Allocation time(ms)')
ax[0].set_xlim(-50, max(df.start.max(), df_free.start.max()) + 50)
ax[0].set_title('Alloc')
ax[0].set_ylim(df1.alloc_time.min(), df1.alloc_time.max() * 2)
ax[1].set_title('Free')
ax[1].set_ylim(df_free.alloc_time.min(), df_free.alloc_time.max() * 2)
fig.tight_layout()
plt.colorbar(density, ax=ax.ravel().tolist(), ticks=cbar_ticks, format=ColorbarFormatter(), pad=0.05)
# plt.show()
fig.savefig(f'plots/{name}.png', dpi=300)
## %%
