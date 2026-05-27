# CW Part4 Package

This package supports the Part4 coursework exercise.

## Included items

- `data/` synthetic dataset with 4 face-format classes:
  - `circlepolis`
  - `rectanglepolis`
  - `trianglepolis`
  - `ovalopolis`
- `train_cnn.py` CPU training script
- `train_cnn_gpu.py` GPU/Colab-friendly training script
- `infer.py` inference script


## Dataset structure

Each class contains **200 images**:
- 140 train
- 30 val
- 30 test

## IMPORTANT

The dataset includes 4 classes, but the model is intentionally trained on **only 2 classes** by default:

- `circlepolis`
- `rectanglepolis`


## Basic commands

### Train on CPU
```bash
python train_cnn.py --data data --epochs 8 --out runs
```

### Train on GPU
```bash
python train_cnn_gpu.py --data data --epochs 10 --out runs_gpu
```

### Run inference
```bash
python infer.py --ckpt runs/best.pt --image data/test/circlepolis/circlepolis_172.png
```
