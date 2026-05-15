from pathlib import Path
import pandas as pd

PROJECT_ROOT = Path(__file__).resolve().parents[1]
INPUT_FILE = PROJECT_ROOT / "data" / "dataset.csv"
OUTPUT_FILE = PROJECT_ROOT / "data" / "dataset_clean_10000.csv"

COLUMN_MAP = {
    "SPEED": "speed_kmh",
    "RPM": "engine_rpm",
    "THROTTLE_POS": "throttle_pos",
    "COOLANT_TEMP": "coolant_temp",
    "FUEL_LEVEL": "fuel_level",
    "INTAKE_TEMP": "intake_air_temp",
}

def classify(row) -> str:
    if row["speed_kmh"] < 25:
        return "SLOW"
    if row["engine_rpm"] > 2500 or row["throttle_pos"] > 45 or row["speed_kmh"] > 80:
        return "AGGRESSIVE"
    return "NORMAL"

def main():
    df = pd.read_csv(INPUT_FILE)

    missing = [c for c in COLUMN_MAP.keys() if c not in df.columns]
    if missing:
        raise ValueError(f"Missing columns in input CSV: {missing}")

    df = df[list(COLUMN_MAP.keys())].rename(columns=COLUMN_MAP)

    for col in COLUMN_MAP.values():
        df[col] = pd.to_numeric(df[col], errors="coerce")

    df = df.dropna().reset_index(drop=True)

    if len(df) > 10000:
        df = df.sample(10000, random_state=42).reset_index(drop=True)

    df["label"] = df.apply(classify, axis=1)

    OUTPUT_FILE.parent.mkdir(parents=True, exist_ok=True)
    df.to_csv(OUTPUT_FILE, index=False)
    print(f"Saved {len(df)} rows to {OUTPUT_FILE}")

if __name__ == "__main__":
    main()