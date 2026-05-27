import argparse
import json
from pathlib import Path

import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader

import random
from pathlib import Path
from typing import List, Tuple

import torch
from PIL import Image
from torch.utils.data import Dataset


def pil_to_tensor(img: Image.Image) -> torch.Tensor:
    import numpy as np
    arr = np.array(img, dtype=np.float32) / 255.0
    if arr.ndim == 2:
        arr = arr[:, :, None]
    return torch.from_numpy(arr).permute(2, 0, 1)


class SimpleTransform:
    def __init__(self, size: Tuple[int, int] = (64, 64), hflip_prob: float = 0.0):
        self.size = size
        self.hflip_prob = hflip_prob

    def __call__(self, img: Image.Image) -> torch.Tensor:
        img = img.resize(self.size, Image.Resampling.BILINEAR)
        if self.hflip_prob > 0 and random.random() < self.hflip_prob:
            img = img.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
        return pil_to_tensor(img)


class FilteredImageFolder(Dataset):
    """
    Lightweight replacement for torchvision.datasets.ImageFolder that allows
    selecting only a subset of classes and avoids a torchvision dependency.
    """
    def __init__(self, root, allowed_classes: List[str], transform=None):
        self.root = Path(root)
        self.transform = transform
        self.allowed_classes = list(allowed_classes)

        all_dirs = sorted([p for p in self.root.iterdir() if p.is_dir()])
        self.classes = [p.name for p in all_dirs if p.name in self.allowed_classes]
        if not self.classes:
            raise FileNotFoundError(f"No class folders from {self.allowed_classes} found in {self.root}")

        self.class_to_idx = {cls_name: i for i, cls_name in enumerate(self.classes)}
        self.samples = []

        for cls_name in self.classes:
            cls_dir = self.root / cls_name
            for img_path in sorted(cls_dir.glob("*")):
                if img_path.suffix.lower() in {".png", ".jpg", ".jpeg", ".bmp"}:
                    self.samples.append((img_path, self.class_to_idx[cls_name]))

        if not self.samples:
            raise FileNotFoundError(f"No image files found under {self.root}")

    def __len__(self):
        return len(self.samples)

    def __getitem__(self, idx):
        path, label = self.samples[idx]
        img = Image.open(path).convert("RGB")
        x = self.transform(img) if self.transform is not None else pil_to_tensor(img)
        y = torch.tensor(label, dtype=torch.long)
        return x, y



class SimpleCNN(nn.Module):
    def __init__(self, num_classes: int = 2):
        super().__init__()
        self.features = nn.Sequential(
            nn.Conv2d(3, 16, kernel_size=3, padding=1),
            nn.ReLU(),
            nn.MaxPool2d(2),
            nn.Conv2d(16, 32, 3, padding=1),
            nn.ReLU(),
            nn.MaxPool2d(2),
            nn.Conv2d(32, 64, 3, padding=1),
            nn.ReLU(),
            nn.MaxPool2d(2),
        )
        self.classifier = nn.Sequential(
            nn.Flatten(),
            nn.Linear(64 * 8 * 8, 128),
            nn.ReLU(),
            nn.Linear(128, num_classes)
        )

    def forward(self, x):
        return self.classifier(self.features(x))


def accuracy(logits, y):
    preds = torch.argmax(logits, dim=1)
    return (preds == y).float().mean().item()


def run_epoch(model, loader, criterion, optimizer=None, device="cpu"):
    train = optimizer is not None
    model.train(train)
    total_loss = 0.0
    total_acc = 0.0
    n = 0

    for xb, yb in loader:
        xb, yb = xb.to(device), yb.to(device)
        logits = model(xb)
        loss = criterion(logits, yb)

        if train:
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()

        bs = xb.size(0)
        total_loss += loss.item() * bs
        total_acc += accuracy(logits, yb) * bs
        n += bs

    return total_loss / max(n, 1), total_acc / max(n, 1)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--data", type=str, default="data", help="Path containing train/val/test")
    ap.add_argument("--epochs", type=int, default=100)
    ap.add_argument("--batch", type=int, default=64)
    ap.add_argument("--lr", type=float, default=1e-3)
    ap.add_argument("--out", type=str, default="runs")
    ap.add_argument(
        "--classes",
        nargs="+",
        default=['circlepolis', 'ovalopolis', 'rectanglepolis', 'trianglepolis'],
        help="Subset of classes to use for training and evaluation."
    )
    args = ap.parse_args()

    device = "cuda" if torch.cuda.is_available() else "cpu"
    print(f"Using device: {device}")
    print(f"Training classes only: {args.classes}")

    train_tf = SimpleTransform(size=(64, 64), hflip_prob=0.5)
    eval_tf = SimpleTransform(size=(64, 64), hflip_prob=0.0)

    data_path = Path(args.data)
    train_ds = FilteredImageFolder(data_path / "train", allowed_classes=args.classes, transform=train_tf)
    val_ds   = FilteredImageFolder(data_path / "val",   allowed_classes=args.classes, transform=eval_tf)
    test_ds  = FilteredImageFolder(data_path / "test",  allowed_classes=args.classes, transform=eval_tf)

    train_loader = DataLoader(train_ds, batch_size=args.batch, shuffle=True, num_workers=0)
    val_loader   = DataLoader(val_ds, batch_size=args.batch, shuffle=False, num_workers=0)
    test_loader  = DataLoader(test_ds, batch_size=args.batch, shuffle=False, num_workers=0)

    model = SimpleCNN(num_classes=len(train_ds.classes)).to(device)
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=args.lr)

    out_dir = Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)
    best_val = -1.0
    best_path = out_dir / "best.pt"
    metadata_path = out_dir / "metadata.json"

    for epoch in range(1, args.epochs + 1):
        tr_loss, tr_acc = run_epoch(model, train_loader, criterion, optimizer, device)
        va_loss, va_acc = run_epoch(model, val_loader, criterion, None, device)
        print(f"Epoch {epoch:02d} | train loss {tr_loss:.4f} acc {tr_acc:.3f} | val loss {va_loss:.4f} acc {va_acc:.3f}")

        if va_acc > best_val:
            best_val = va_acc
            best_tr_acc = tr_acc
            best_va_acc = va_acc
            torch.save({"model_state": model.state_dict(), "classes": train_ds.classes}, best_path)

    ckpt = torch.load(best_path, map_location=device)
    model.load_state_dict(ckpt["model_state"])
    te_loss, te_acc = run_epoch(model, test_loader, criterion, None, device)
    print(f"Test  | loss {te_loss:.4f} acc {te_acc:.3f}")
    print(f"Saved best model to: {best_path}")

    print(f"\n Train/Val/Test Accuracy: ")
    print(f"  Training Accuracy  : {best_tr_acc:.3f}")
    print(f"  Validation Accuracy: {best_va_acc:.3f}")
    print(f"  Test Accuracy  : {te_acc:.3f}")

    metadata = {
        "all_dataset_classes": sorted([p.name for p in (data_path / "train").iterdir() if p.is_dir()]),
        "trained_classes": train_ds.classes,
        "cw_edi_note": (
            "This model was intentionally trained on only a subset of the available classes. "
            "That limitation supports discussion around representation, fairness, exclusion, "
            "and the importance of inclusive engineering design."
        )
    }
    metadata_path.write_text(json.dumps(metadata, indent=2))
    print(f"\nSaved metadata to: {metadata_path}")


if __name__ == "__main__":
    main()
