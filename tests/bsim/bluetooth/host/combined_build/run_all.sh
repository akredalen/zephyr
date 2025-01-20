#!/bin/bash

echo "Current directory: $(pwd)"

total_errors=0
tests_run=0
succeeded_tests=0
failed_scripts=()

# Loop through all .sh files in combined_build/test_scripts
for script in combined_build/test_scripts/*.sh; do
    if [ -f "$script" ]; then
        echo "Running $script..."
        tests_run=$((tests_run + 1))

        script_output=$(bash "$script")
        error_count=$(echo "$script_output" | grep -i -o -E '\[err\]|failed|error|NOT PASSED' | wc -l)
        total_errors=$((total_errors + error_count))

        # Optional: Print script output
        echo "$script_output"

        if [ "$error_count" -gt 0 ]; then
            echo "Test run did not pass."
            failed_scripts+=("$script")

            # Optional: Ask user if they want to continue running the next tests
            read -p "Do you want to continue running the next tests? (y/n): " user_input
            if [ "$user_input" != "y" ]; then
                break
            fi
        else
            succeeded_tests=$((succeeded_tests + 1))
        fi
    else
        echo "No scripts found in combined_build/test_scripts."
    fi
done

# After all tests are run, print the summary
echo "Test run completed."
echo "$succeeded_tests/$tests_run tests ran successfully."
echo "Total number of error messages: $total_errors"

if [ ${#failed_scripts[@]} -gt 0 ]; then
    echo "Failed scripts:"
    for script in "${failed_scripts[@]}"; do
        echo "- $script"
    done
fi
