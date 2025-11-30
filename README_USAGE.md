# CELP Decoder Usage Guide

## Issue
The command `./celp decode ./enc.celp ./dec.raw` resulted in a segmentation fault because the file `enc.celp` does not exist in the current directory.

## Working Examples

The following commands work correctly:

```bash
# Decode an existing encoded file
./celp decode test_encoded.celp decoded_output.raw

# Decode the other test file
./celp decode test_test.celp decoded_output2.raw
```

## To encode a file first:
```bash
# Encode a raw audio file (like test.raw)
./celp encode test.raw new_encoded.celp
```

## Then decode it:
```bash
# Decode the newly encoded file
./celp decode new_encoded.celp decoded_from_new.raw
```

## Available test files:
- `test.raw` - Sample raw audio file
- `test_encoded.celp` - Pre-encoded CELP file
- `test_test.celp` - Another pre-encoded CELP file
- Various decoded output files from testing