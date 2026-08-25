#!/bin/bash
# This script is used to collect dynamic  profiling data for AMD GPUs using rocprof-compute and perform various analyses on the collected data.
echo "======================================================================================================"
if [ "$dry_run" = false ]; then
  echo "==== collecting kernel profiling data"

  # creating profiling data
  rm -rf "workloads/$executable_filename" # Clear possible previous workloads folder TODO: Can be uncommented when not debugging
  rocprof-compute profile --name "$executable_filename" --no-roof --quiet -- "$executable" "$args"

  mkdir -p metrics

  gpu_name=$(ls "workloads/$executable_filename/")
  # get the number of kernels and their respective kernel names in the executable
  analysis_output=$(rocprof-compute analyze -p "workloads/$executable_filename/$gpu_name/" --list-stats)

  #  Depending on the ROCm compiler/profiler version, rocprof-compute --list-stats may report an AMDHSA kernel-descriptor symbol whose demangled name ends with"[clone .kd]". 
  #  Parse the complete Kernel_Name table cell instead of assuming that the kernel name ends with ")". see https://llvm.org/docs/AMDGPUUsage.html?#amdhsa-kernel-name
  printf '%s\n' "$analysis_output" |
      perl -ne '
          if (/^\s*Detected Kernels\b/) {
              $in_kernel_table = 1;
              next;
          }

          if ($in_kernel_table && /^\s*╘/) {
              $in_kernel_table = 0;
              next;
          }

          if ($in_kernel_table && /^\s*│\s*(\d+)\s*│\s*([^│]+?)\s*│\s*$/) {
              print "$1,$2\n";
          } 
          ' > metrics/kernel_list.csv

  if [ ! -s metrics/kernel_list.csv ]; then
      echo "ERROR: Failed to extract kernels from rocprof-compute --list-stats."
      exit 1
  fi

  # Need to move everything into metrics folder because the analyze mode cant save the result in a directory
  # and also cant access parent directories
  mv "workloads" "metrics/workloads"
  cd metrics || exit

  # Determine if the current version of rocprof-compute produces legacy analyze output (version 3.0.0) or newer output formats.
  rocprof_compute_version_output=$(rocprof-compute --version 2>&1)
  case "$rocprof_compute_version_output" in
      *"rocprofiler-compute version: 3.0.0"*)
          legacy_analyze_output=true
          ;;
      *)
          legacy_analyze_output=false
          ;;
  esac
  printf 'Use legacy analyze output: %s\n' "$legacy_analyze_output"

  # Use different file descriptor (3) for the loop because rocprof-compute consumes/reads stdin resulting in only one kernel analyzed
  exec 3< kernel_list.csv

  # analyze each kernel separately
  while IFS=, read -r -u 3 kernel_number raw_kernel_name; do
      # Remove an optional AMDHSA descriptor suffix from the user-facing name.
      display_kernel_name=$(printf '%s' "$raw_kernel_name" | sed 's/[[:space:]]*\[clone \.kd\][[:space:]]*$//')

      # Replace spaces and special characters with underscores.
      sanitized_kernel_name=$(printf '%s' "$display_kernel_name" | tr -c 'a-zA-Z0-9' '_' | tr -s '_' | sed 's/^_*//;s/_*$//')

      output_file="${executable_filename}_${sanitized_kernel_name}_metrics"

      analyze_arguments=(
          analyze
          -p "workloads/$executable_filename/$gpu_name"
          -b
          7.2.4
          10.1.6
          10.2.14
          11.2.5
          12.1.0
          12.1.1
          12.1.3
          12.2.5
          15.1.9
          15.1.13
          16.3.3
          16.3.5
          17.2.7
          17.3.1
          17.3.4
          17.5.10
          -n per_kernel
          -k "$kernel_number"
      )

      if [ "$legacy_analyze_output" = true ]; then
          # rocprofiler-compute 3.0.0 prints the report to stdout.
          rocprof-compute "${analyze_arguments[@]}" > "${output_file}.txt"
      else
          # Newer versions can create and name the text report themselves.
          rocprof-compute "${analyze_arguments[@]}" \
              --output-name "$output_file" \
              --output-format txt \
              > /dev/null
      fi
  done

  exec 3<&- # release file descriptor 3
  cd .. || exit # go back to previous work directory



  ##### Creating file containing PC sampling data when supported by the gpu TODO Could be moved into metrics collection when pc sampling works without problems #####
  stochastic_gpus=$(rocprofv3 -L | grep -A 7 "GPU\s*:" | grep  "Method\s*:\s*stochastic")
  stochastic_gpus="" # TODO remove when rocprof-compute bug is fixed - doesnt collect data right now and data fields are just empty
  if [ -n "$stochastic_gpus" ]; then
      echo "==== collecting PC sampling data"

      mkdir pc_samples
      # Collect pc sampling data with the command from the docs - 128 cycles because nvidia version uses the same amount
      rocprof-compute profile -n pc_test -b 21 --no-roof --pc-sampling-method stochastic --pc-sampling-interval 128 -VVV -- "$executable" "$args"
      mv "workloads" "pc_samples/workloads"
      cd pc_samples || exit

      # Use different file descriptor (3) for the loop because rocprof-compute consumes/reads stdin resulting in only one kernel analyzed
      exec 3< ../metrics/kernel_list.csv
      # analyze each kernel separately
      while IFS=, read -r -u 3 kernel_number kernel_name; do
          # replace spaces and special characters in kernel_name for the output filename
          sanitized_kernel_name=$(printf '%s' "$kernel_name" | tr -c 'a-zA-Z0-9' '_' | sed 's/^_//;s/_$//')
          output_file="${executable_filename}_${sanitized_kernel_name}_pc_samples"

          # execute the per-kernel analysis
          rocprof-compute analyze -p "workloads/$executable_filename/$gpu_name" -b 21 -k "$kernel_number" \
          --output-name "$output_file" --output-format txt --pc-sampling-sorting-type offset
      done
      exec 3<&- # release file descriptor 3
      cd .. || exit
  else
      echo "==== no GPU on this system supports hardware-based (stochastic) PC sampling"
  fi
fi

#### Perform analysis on the collected data ####
cd "${gpuscout_dir}"/analysis_amd || exit

assembly="${gpuscout_tmp_dir}/${executable_filename}.s"
metrics_dir="${gpuscout_tmp_dir}/metrics"
livereg_dir="${gpuscout_tmp_dir}/livereg"

if [ "$performance_mode" = false ]; then
  ./analysis_register_spilling  "${assembly}" "${metrics_dir}" "${livereg_dir}" "${json}" "${gpuscout_output_dir}"

  ./analysis_restrict "${assembly}" "${metrics_dir}" "${livereg_dir}" "${json}" "${gpuscout_output_dir}"

  ./analysis_vectorized_load "${assembly}" "${metrics_dir}" "${livereg_dir}" "${json}" "${gpuscout_output_dir}"

  ./analysis_atomic_instruction "${assembly}" "${metrics_dir}" "${json}" "${gpuscout_output_dir}"

  ./analysis_wavefront_divergence "${assembly}" "${metrics_dir}" "${json}" "${gpuscout_output_dir}"

  ./analysis_shared_memory "${assembly}" "${metrics_dir}" "${json}" "${gpuscout_output_dir}"

  ./analysis_datatype_conversion "${assembly}" "${metrics_dir}" "${json}" "${gpuscout_output_dir}"

  ./analysis_deadlock_detection "${assembly}" "${json}" "${gpuscout_output_dir}"

else
  # Use Multi-Threading for faster analysis -> each analysis within its own thread
  analysis_logs_dir="${gpuscout_tmp_dir}/analysis_tmp_outputs"
  mkdir -p "$analysis_logs_dir"
  declare -a names=() pids=()

  all_log="${analysis_logs_dir}/all_analyses.log"
  : >"$all_log"

  run() {
    local name="$1"; shift
    (
      "$@" # Execute the passed command
      rc=$? # Captures the exit code
      exit "$rc"
    ) >>"$all_log" 2>&1 &
    pids+=("$!")
  }

  # Launch all analyses in parallel
  run register_spilling      ./analysis_register_spilling  "$assembly" "$metrics_dir" "$livereg_dir" "$json" "$gpuscout_output_dir"
  run restrict               ./analysis_restrict           "$assembly" "$metrics_dir" "$livereg_dir" "$json" "$gpuscout_output_dir"
  run vectorized_load        ./analysis_vectorized_load    "$assembly" "$metrics_dir" "$livereg_dir" "$json" "$gpuscout_output_dir"
  run atomic_instruction     ./analysis_atomic_instruction "$assembly" "$metrics_dir" "$json" "$gpuscout_output_dir"
  run wavefront_divergence   ./analysis_wavefront_divergence "$assembly" "$metrics_dir" "$json" "$gpuscout_output_dir"
  run shared_memory          ./analysis_shared_memory      "$assembly" "$metrics_dir" "$json" "$gpuscout_output_dir"
  run datatype_conversion    ./analysis_datatype_conversion "$assembly" "$metrics_dir" "$json" "$gpuscout_output_dir"
  run deadlock_detection     ./analysis_deadlock_detection "$assembly" "$json" "$gpuscout_output_dir"

  # Wait and collect exit codes
  declare -a rc=()
  for i in "${!pids[@]}"; do
    if wait "${pids[$i]}"; then
      rc[$i]=0
    else
      rc[$i]=$?
    fi
  done

  # Print captured outputs after all are done
  cat "$all_log"

fi


# output all used files in JSON format
if [ "$json" = true ]; then

    echo "======================================================================================================"
    echo "==== generating JSON output"
    # ./save_to_json ${gpuscout_output_dir} ${gpuscout_tmp_dir}/result-${run_prefix} ${gpuscout_tmp_dir}/nvdisasm-executable-${executable_filename}-sass.txt ${gpuscout_tmp_dir}/nvdisasm-registers-executable-${executable_filename}-sass.txt ${gpuscout_tmp_dir}/nvdisasm-executable-${executable_filename}-ptx.txt ${gpuscout_tmp_dir}/pcsampling_${executable_filename}.txt
    ./save_to_json ${gpuscout_output_dir} ${gpuscout_tmp_dir}/result-${run_prefix} "${assembly}" "${metrics_dir}" "$livereg_dir"
fi

echo "======================================================================================================"
echo "Time for Static Code Analysis: ${static_time}s"
if [ "$dry_run" = false ]; then
    echo "Time for PC Sampling:          ${pcsampling_time}s"
    echo "Time for Metrics Collection:   ${metrics_time}s"
fi
echo "Time for Merging Analysis:     ${analysis_time}s"
echo "======================================================================================================"

cd ..
