import matplotlib.pyplot as plt
import numpy as np

epochs = [8, 16, 32]
training   = [0.504, 0.871, 0.986]
validation = [0.750, 0.950, 1.000]
test       = [0.717, 0.817, 0.983]

x = np.arange(3)
width = 0.25
labels = ['Training', 'Validation', 'Test']
colors = ['red', 'blue', 'green']

fig, ax = plt.subplots(figsize=(8, 5))

for i, (epoch, t, v, te, color) in enumerate(zip(epochs, training, validation, test, colors)):
    data = [t, v, te]
    ax.bar(x + i * width, data, width, label=f'Epoch {epoch}', color=color, edgecolor='white', alpha=0.85)

ax.set_title('Accuracy across different Number of Epochs', fontsize=13, fontweight='bold')
ax.set_ylabel('Accuracy')
ax.set_ylim(0, 1.1)
ax.set_xticks(x + width)
ax.set_xticklabels(labels)
ax.legend(title='Epochs', loc='upper left', bbox_to_anchor=(1, 1), borderaxespad=0)
plt.tight_layout(rect=[0, 0, 0.88, 1])
ax.yaxis.set_major_formatter(plt.FuncFormatter(lambda v, _: f'{v:.2f}'))

plt.tight_layout()
plt.savefig('accuracy_epochs.png', dpi=150, bbox_inches='tight')
plt.show()