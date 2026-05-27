import argparse
import json
from pathlib import Path

import torch
import torch.nn as nn
from PIL import Image

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
    def __init__(self, num_classes=2):
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


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ckpt", type=str, default="runs/best.pt")
    ap.add_argument("--image", type=str, required=True)
    ap.add_argument(
        "--threshold",
        type=float,
        default=0.65,
        help="If max probability is below this, print a warning that the face may not belong to known classes."
    )
    args = ap.parse_args()

    device = "cuda" if torch.cuda.is_available() else "cpu"
    ckpt = torch.load(args.ckpt, map_location=device)
    classes = ckpt["classes"]

    model = SimpleCNN(num_classes=len(classes)).to(device)
    model.load_state_dict(ckpt["model_state"])
    model.eval()

    tf = SimpleTransform(size=(64, 64), hflip_prob=0.0)
    img = Image.open(args.image).convert("RGB")
    x = tf(img).unsqueeze(0).to(device)

    with torch.no_grad():
        logits = model(x)
        probs = torch.softmax(logits, dim=1).squeeze(0)
        idx = int(torch.argmax(probs).item())
        max_prob = float(probs[idx].item())

    print("Prediction:", classes[idx])
    print("Confidence:", f"{max_prob:.3f}")

    if max_prob < args.threshold:
        print("Warning: low confidence :-(. The image may belong to a face type that was not represented in training.")

    for c, p in zip(classes, probs.tolist()):
        print(f"  {c:15s} {p:.3f}")

    metadata_path = Path(args.ckpt).parent / "metadata.json"
    if metadata_path.exists():
        meta = json.loads(metadata_path.read_text())
        print("\nTraining scope:")
        print("  trained classes :", meta.get("trained_classes"))
        print("  dataset classes :", meta.get("all_dataset_classes"))
        print("  note            :", meta.get("cw_edi_note"))


if __name__ == "__main__":
    main()
