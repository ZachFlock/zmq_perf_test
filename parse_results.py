import os
import glob
import re

def parse_file(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
        
    # Extract values using regex
    patterns = {
        'min': r'Min:\s+(\d+)',
        'mean': r'Mean:\s+(\d+\.?\d*)',
        'median': r'Median:\s+(\d+)',
        'p95': r'P95:\s+(\d+)',
        'p99': r'P99:\s+(\d+)',
        'max': r'Max:\s+(\d+)',
        'dropped': r'Dropped messages:\s+(\d+)'
    }
    
    results = {}
    for metric, pattern in patterns.items():
        match = re.search(pattern, content)
        if match:
            value = float(match.group(1))
            results[metric] = value
    
    return results

def find_worst_case():
    # Find all .out files in the current directory and subdirectories
    out_files = glob.glob('*.out', recursive=True)
    
    if not out_files:
        print("No .out files found in the directory!")
        return
    
    # Initialize worst case values
    worst_case = {
        'min': float('-inf'),
        'mean': float('-inf'),
        'median': float('-inf'),
        'p95': float('-inf'),
        'p99': float('-inf'),
        'max': float('-inf'),
        'dropped': float('-inf')
    }
    
    # Track which file produced each worst case
    worst_case_files = {}
    
    # Process each file
    for file_path in out_files:
        try:
            results = parse_file(file_path)
            
            # Update worst case values
            for metric, value in results.items():
                if value > worst_case[metric]:
                    worst_case[metric] = value
                    worst_case_files[metric] = file_path
                    
        except Exception as e:
            print(f"Error processing {file_path}: {e}")
    
    # Print results
    print("\nWorst Case Results:")
    print("-" * 50)
    for metric in worst_case:
        value = worst_case[metric]
        file = worst_case_files.get(metric, "N/A")
        print(f"{metric.capitalize():6s}: {value:10.1f} µs (from {os.path.basename(file)})")

if __name__ == "__main__":
    find_worst_case()