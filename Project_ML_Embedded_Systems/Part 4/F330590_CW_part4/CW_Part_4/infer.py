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
    ap.add_argument("--image", type=str, default=None, help="Single image path (optional)")
    ap.add_argument("--test_dir", type=str, default=None, help="Directory of class subfolders for batch inference")
    ap.add_argument(
        "--threshold",
        type=float,
        default=0.65,
        help="If max probability is below this, print a warning."
    )
    args = ap.parse_args()

    device = "cuda" if torch.cuda.is_available() else "cpu"
    ckpt = torch.load(args.ckpt, map_location=device)
    classes = ckpt["classes"]

    model = SimpleCNN(num_classes=len(classes)).to(device)
    model.load_state_dict(ckpt["model_state"])
    model.eval()

    tf = SimpleTransform(size=(64, 64), hflip_prob=0.0)
    def infer_single(img_path):
        img = Image.open(img_path).convert("RGB")
        x = tf(img).unsqueeze(0).to(device)

        with torch.no_grad():
            logits = model(x)
            probs = torch.softmax(logits, dim=1).squeeze(0)
            idx = int(torch.argmax(probs).item())
            max_prob = float(probs[idx].item())
        return classes[idx], max_prob, probs.tolist()

    if args.image:
        predicted_class, max_prob, probs_list = infer_single(args.image)
        print("Prediction:", predicted_class)
        print("Confidence:", f"{max_prob:.3f}")
        if max_prob < args.threshold:
            print("Warning: low confidence. The image may not belong to a known class.")
        for c, p in zip(classes, probs_list):
            print(f"  {c:15s} {p:.3f}")

    elif args.test_dir:
        test_path = Path(args.test_dir)
        all_test_classes = sorted([p.name for p in test_path.iterdir() if p.is_dir()])

        class_correct = {c: 0 for c in all_test_classes}
        class_total = {c: 0 for c in all_test_classes}
        class_pred_counts = {c: {p: 0 for p in classes} for c in all_test_classes}
        total_correct = 0
        total_images = 0

        for true_class in all_test_classes:
            class_dir = test_path / true_class
            img_paths = [p for p in sorted(class_dir.glob("*"))]
            print(f"\nClass: {true_class}")

            for img_path in img_paths:
                predicted_class, max_prob, probs_list = infer_single(img_path)
                is_correct = predicted_class == true_class

                class_total[true_class] += 1
                class_pred_counts[true_class][predicted_class] += 1
                if is_correct:
                    class_correct[true_class] += 1
                    total_correct += 1
                total_images += 1

                if is_correct:
                    print(f"{img_path.name} | Confidence = {max_prob:.3f} | Correct")
                else:
                    print(
                        f"{img_path.name} | Confidence = {max_prob:.3f} | Incorrect predicted class was {predicted_class}")

        print("\nInference results for the entire dataset:\n")
        print(f"{'Class':<20} {'Correct':>8} {'Total':>8} {'Accuracy':>10}")
        for c in all_test_classes:
            if class_total[c] == 0:
                continue
            acc = class_correct[c] / class_total[c]
            print(f"{c:<20} {class_correct[c]:>8} {class_total[c]:>8} {acc:>10.3f}")
        overall_acc = total_correct / max(total_images, 1)
        print(f"\n{'Overall':<20} {total_correct:>8} {total_images:>8} {overall_acc:>10.3f}\n")

if __name__ == "__main__":
    main()
