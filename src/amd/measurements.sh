#!/bin/bash

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

cd ..
