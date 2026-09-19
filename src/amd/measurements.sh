#!/bin/bash
# This script is used to collect dynamic profiling data for AMD GPUs using rocprof-compute and perform various analyses on the collected data.
echo "======================================================================================================"
########################################################################
# Analysis on assmble files 
########################################################################
if [ ! -s "$assembly" ]; then
    echo "ERROR: AMD assembly file was not created or is empty: $assembly" >&2
    exit 2
fi
automatic_mode=false
if [ -z "${analysis_arg:-}" ]; then
  #### Automatic mode ####
  # enable analyses only when their static bottleneck occurrence has been detected.
  automatic_mode=true
  enabled_analyses=()
  echo "AMD analysis selection mode: automatic (static-triggered)"

  static_detect_analyses=(
    register_spilling
    atomic_instruction
    datatype_conversion
    deadlock_detection
    restrict
    shared_memory
    vectorized_load
    wavefront_divergence
  )
  
  start_static_detect=$(date +%s.%N)
  selected_static_detect_time=0

  for analysis in "${static_detect_analyses[@]}"; do
    detector="${gpuscout_dir}/analysis_amd/parser_amdgcn_${analysis}"
    detector_start=$(date +%s.%N)

    if "$detector" "$assembly"; then
      detector_rc=0
    else
      detector_rc=$?
    fi

    detector_end=$(date +%s.%N)
    detector_time=$(awk "BEGIN {print $detector_end - $detector_start}")
    echo "Static detection time for $analysis: ${detector_time}s"

    if [ "$detector_rc" -eq 0 ]; then
      enabled_analyses+=("$analysis")
      selected_static_detect_time=$(awk "BEGIN {print $selected_static_detect_time + $detector_time}")
      echo "Detected bottleneck candidate: $analysis"
    elif [ "$detector_rc" -eq 1 ]; then
      echo "No bottleneck candidate detected: $analysis"
    else
      echo "ERROR: Static detection failed for $analysis." >&2
      exit "$detector_rc"
    fi
  done
  echo "Static detection time of selected analyses: ${selected_static_detect_time}s"
else
  #### Manual mode ####
  # the existing CLI-based bottleneck analysis selection.
  automatic_mode=false
  parse_csv_list "${analysis_arg}" "analysis"
  enabled_analyses=("${parsed_csv_list[@]}")
  echo "AMD analysis selection mode: manual"

  for analysis in "${enabled_analyses[@]}"; do
    parser="${gpuscout_dir}/analysis_amd/parser_amdgcn_${analysis}"

    if ! "$parser" "$assembly"; then
      parser_rc=$?
      # Generate the static JSON required by each manually selected analysis.
      # Do not use the detection outcome to alter enabled_analyses.
      if [ "$parser_rc" -ne 1 ]; then
        echo "ERROR: Static parser failed for $analysis." >&2
        exit "$parser_rc"
      fi
    fi
  done
fi
end_static_detect=$(date +%s.%N)
static_detect_time=$(awk "BEGIN {print $end_static_detect - $start_static_detect}")

echo "Selected AMD analyses: $(join_by_comma "${enabled_analyses[@]}")"

#### Build candidate kernel->analyses mapping for automatic mode ####
candidate_kernels=()
declare -A kernel_analyses=()
if [ "$automatic_mode" = true ]; then
  if ! command -v jq >/dev/null 2>&1; then
    echo "ERROR: jq is required to read AMD static analysis results." >&2
    exit 1
  fi

  for analysis in "${enabled_analyses[@]}"; do
    static_result_file="${gpuscout_tmp_dir}/static_results/${analysis}.json"
    if [ ! -s "$static_result_file" ]; then
      echo "ERROR: Missing static result for analysis: $analysis" >&2
      exit 1
    fi

    while IFS= read -r kernel; do
      [ -z "$kernel" ] && continue
      # First time this candidate kernel is encountered.
      if [ -z "${kernel_analyses[$kernel]+x}" ]; then
        candidate_kernels+=("$kernel")
        kernel_analyses["$kernel"]="$analysis"
      else
        kernel_analyses["$kernel"]+=" $analysis"
      fi
    done < <( jq -r '.candidate_kernels[].demangled' "$static_result_file")
  done

  echo "==== Candidate kernel analysis mapping"
  for kernel in "${candidate_kernels[@]}"; do
    echo "Kernel: $kernel"
    echo "  analyses: ${kernel_analyses[$kernel]}"
  done
fi

########################################################################
# Live Register
########################################################################
#### Check if any of the selected analyses require register pressure information ####
livereg_required=false
_livereg_analyses=(
  register_spilling
  restrict
  vectorized_load
)

for analysis in "${enabled_analyses[@]}"; do
  if array_contains "$analysis" "${_livereg_analyses[@]}"; then
    livereg_required=true
    break
  fi
done

# creating file containing register pressure information
mkdir -p "$livereg_dir"
livereg_time=0
if [ "$livereg_required" = true ]; then
  echo "==== creating file containing register pressure information "
  start_livereg=$(date +%s.%N)
  "$rga" -s bin --livereg livereg/9_73_23_42_${executable_filename}-vgpr.txt --co "$object_file" # 9_73_23_42 to make kernel name extraction easier
  "$rga" -s bin --livereg-sgpr livereg/9_73_23_42_${executable_filename}-sgpr.txt --co "$object_file"
  end_livereg=$(date +%s.%N)
  livereg_time=$(awk "BEGIN {print $end_livereg - $start_livereg}")
else
  echo "Skipping register pressure information collection: selected analyses do not require it."
fi

########################################################################
# Build metric requirements
########################################################################
if [ "$dry_run" = false ]; then
  # The IDs must match the fields parsed in parser_metrics.hpp
  _metrics_register_spilling=(
    7.2.4   # Wavefront dependency wait cycles per cycle
    # 15.1.9  # Spill/stack instructions per kernel
    15.2.5  # Spill/stack instructions per kernel for rocprof-compute 3.3.0 (15.1.9 is not available in this version)
    # 15.1.13 # Spill/stack processing cycles per kernel
    15.3.0 # Spill/stack processing cycles per kernel for rocprof-compute 3.3.0 (15.1.13 is not available in this version)
    16.3.5  # VL1D cache line hit rate
    17.3.1  # Total incoming requests to L2 cache per kernel
  )
  _metrics_restrict=()
  _metrics_vectorized_load=(
    7.2.4  # Wavefront dependency wait cycles per kernel
  )
  _metrics_atomic_instruction=(
    7.2.4   # Wavefront dependency wait cycles per kernel
    12.2.5  # LDS atomic return cycles per cycles
    16.3.3  # VL1D Cache Atomic Requests per kernel
    17.2.7  # Percentage of atomic write requests in write requests generated by the L2 cache to Infinity Fabric
    17.3.4  # L2 Cache Atomic Requests per kernel
    # 17.5.10 # Atomic requests towards Infinity Fabric per kernel
    17.6.10 # Atomic requests towards Infinity Fabric per kernel for rocprof-compute 3.3.0 (17.5.10 is not available in this version)
    
  )
  _metrics_wavefront_divergence=(
    10.1.6  # Branch instruction count per kernel
    11.2.5  # Branch unit utilization percentage
  )
  _metrics_shared_memory=(
    7.2.4   # Wavefront dependency wait cycles per kernel
    12.1.0  # Percentage of the kernel's execution time during which the LDS was actively processing operations, including loads, stores, atomics, and shuffle operations.
    12.1.1  # Percentage of CU cycles during which the schedulers issued LDS instructions, averaged across the SIMDs over the lifetime of the kernel.
    12.1.3  # Percentage of LDS service cycles spent resolving bank conflicts relative to the cycles required to serve the same accesses without bank conflicts.
  )
  _metrics_datatype_conversion=(
    10.2.14 # Type conversion instructions issued to the VALU per kernel
  )
  _metrics_deadlock_detection=()
  _metrics_json_export=(
    7.2.4
    10.1.6
    10.2.14
    11.2.5
    12.1.0
    12.1.1
    12.1.3
    12.2.5
    15.2.5 # 15.1.9
    15.3.0 # 15.1.13
    16.3.3
    16.3.5
    17.2.7
    17.3.1
    17.3.4
    17.6.10 #17.5.10
  )
  # Map requested metrics to their corresponding hardware blocks for rocprof-compute versions that do not support metric-ID filtering.
  declare -A _metric_profile_block=(
    [7.2.4]="SQ"
    [10.1.6]="SQ"
    [10.2.14]="SQ"
    [11.2.5]="SQ"
    [12.1.0]="SQ"
    [12.1.1]="SQ"
    [12.1.3]="SQ"
    [12.2.5]="SQ"
    [15.2.5]="TA"
    [15.3.0]="TA"
    [16.3.3]="TCP"
    [16.3.5]="TCP"
    [17.2.7]="TCC"
    [17.3.1]="TCC"
    [17.3.4]="TCC"
    [17.6.10]="TCC"
  )

  if [ "$automatic_mode" = true ]; then
    #### Automatic mode: build kernel -> metrics mapping ####
    declare -A kernel_metrics=()
    _add_kernel_metric()
    {
      local kernel="$1"
      local metric="$2"
      local current_metrics="${kernel_metrics[$kernel]:-}"

      # Metric IDs contain no spaces, therefore a space-separated list is sufficient here.
      case " $current_metrics " in
        *" $metric "*)
          ;;
        *)
          if [ -n "$current_metrics" ]; then
            kernel_metrics["$kernel"]+=" $metric"
          else
            kernel_metrics["$kernel"]="$metric"
          fi
          ;;
      esac
    }
    # Add all metrics required by one analysis to the metric set of one kernel.
    # target_array_name contains the NAME of an analysis-specific metric array,
    # e.g. "_metrics_register_spilling".
    _add_analysis_metrics_to_kernel()
    {
      local target_kernel="$1"
      local target_array_name="$2"
      # target_array_name="_metrics_register_spilling"
      # metrics_ref -> _metrics_register_spilling
      local -n metrics_ref="$target_array_name"
      local metric

      for metric in "${metrics_ref[@]}"; do
        _add_kernel_metric "$target_kernel" "$metric"
      done
    }

    for kernel in "${candidate_kernels[@]}"; do
      for analysis in ${kernel_analyses[$kernel]}; do
        array_name="_metrics_${analysis}"
        _add_analysis_metrics_to_kernel "$kernel" "$array_name"
      done
      # Additional metrics required for JSON output.
      # These metrics are not needed for bottleneck detection itself, but are required by the GUI. #TODO: should check metrics exported by json is really needed for each kernel
      if [ "$json" = true ]; then
        for metric in "${_metrics_json_export[@]}"; do
          _add_kernel_metric "$kernel" "$metric"
        done
      fi
    done

    echo "==== AMD kernel profiling plan"
    amd_metrics_required=false
    for kernel in "${candidate_kernels[@]}"; do
      echo "Kernel: $kernel"
      echo "  analyses: ${kernel_analyses[$kernel]}"
      echo "  metrics : ${kernel_metrics[$kernel]:-(none)}"

      if [ -n "${kernel_metrics[$kernel]:-}" ]; then
        amd_metrics_required=true
      fi
    done
  else
    #### Manual mode: Per-analysis metric requirements ####
    # Build a de-duplicated list of metric IDs required by the enabled analyses.
    _metrics_set=()
    _add_metrics()
    {
      local metric
      for metric in "$@"; do
        if ! array_contains "$metric" "${_metrics_set[@]}"; then
          _metrics_set+=("$metric")
        fi
      done
    }

    for analysis in "${enabled_analyses[@]}"; do
      if ! array_contains "${analysis}" "${valid_analyses[@]}"; then
        echo "ERROR: Unknown analysis name in enabled_analyses: $analysis"
        exit 1
      fi

      array_name="_metrics_${analysis}"
      declare -n current_metrics="${array_name}"

      _add_metrics "${current_metrics[@]}"
    done

    if [ "$json" = true ]; then
      _add_metrics "${_metrics_json_export[@]}"
    fi

    amd_metrics_required=false
    if [ "${#_metrics_set[@]}" -gt 0 ]; then
      amd_metrics_required=true
    fi
  fi

########################################################################
# Metric profiling
########################################################################
  mkdir -p metrics
  if [ "$amd_metrics_required" = true ]; then
    echo "==== collecting kernel profiling data"
    start_metrics=$(date +%s.%N)
    profile_help=$(rocprof-compute profile --help 2>&1)
    # Determine whether the current rocprof-compute version supports filtering directly by metric IDs or only by hardware blocks.
    if printf '%s\n' "$profile_help" | grep -Eqi 'metric([[:space:]]+|-)id'; then
      metric_id_filter_supported=true
      echo "AMD profile filter mode: metric IDs"
    else
      metric_id_filter_supported=false
      echo "AMD profile filter mode: hardware blocks"
    fi

    #### Build rocprof-compute profile filter arguments ####
    _build_profile_filter_arguments()
    {
      local -a requested_metrics=("$@")
      profile_filter_arguments=()
      # filter by metric ID
      if [ "$metric_id_filter_supported" = true ]; then
        profile_filter_arguments=(
          -b
          "${requested_metrics[@]}"
        )
      # filter by metric block
      else
        local -a profile_blocks=()
        local metric_id
        local block
        for metric_id in "${requested_metrics[@]}"; do
          block="${_metric_profile_block[$metric_id]:-}"
          if [ -z "$block" ]; then
            echo "ERROR: No AMD hardware block mapping for metric ID $metric_id" >&2
            return 1
          fi
          if ! array_contains "$block" "${profile_blocks[@]}"; then
            profile_blocks+=("$block")
          fi
        done
        profile_filter_arguments=(
          -b
          "${profile_blocks[@]}"
        )
      fi
    }

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

    #### Automatic mode #####
    if [ "$automatic_mode" = true ]; then
      rm -rf workloads
      rm -rf metrics/workloads
      declare -A kernel_profile_name=()
      profile_index=0
      # profile each candidate kernel given by the correspponding metrics
      for kernel in "${candidate_kernels[@]}"; do
        metrics_string="${kernel_metrics[$kernel]:-}"
        # Analyses such as deadlock detection or restrict may not require any dynamic hardware metrics.
        if [ -z "$metrics_string" ]; then
          echo  "Skipping metric collection for kernel '$kernel': no metrics required."
          continue
        fi
        read -r -a current_kernel_metrics <<< "$metrics_string"
        if ! _build_profile_filter_arguments  "${current_kernel_metrics[@]}"; then
          exit 1
        fi

        profile_name="${executable_filename}_candidate_${profile_index}"
        kernel_profile_name["$kernel"]="$profile_name"

        echo "==== profiling candidate kernel"
        echo "Kernel:  $kernel"
        echo "Metrics: ${current_kernel_metrics[*]}"
        # creating profiling data
        rocprof-compute profile --name "$profile_name" -k "$kernel" --no-roof --quiet "${profile_filter_arguments[@]}" -- "$executable" "$args"

        profile_index=$((profile_index + 1))
      done

      # Analyze the already kernel-filtered workloads
      if [ "$profile_index" -gt 0 ]; then
        mv workloads metrics/workloads
        cd metrics || exit
        for kernel in "${candidate_kernels[@]}"; do
          profile_name="${kernel_profile_name[$kernel]:-}"
          # Skip metrics anaylze when analysis for kernel does not require any metrics
          if [ -z "$profile_name" ]; then
            continue
          fi
          metrics_string="${kernel_metrics[$kernel]}"
          read -r -a current_kernel_metrics <<< "$metrics_string"

          if [ ! -d "workloads/$profile_name" ]; then
            echo "ERROR: Can not find metrics directory."
            exit 1
          fi
          gpu_name=$(ls "workloads/$profile_name/")
          workload_path="workloads/$profile_name/$gpu_name"

          sanitized_kernel_name=$(printf '%s' "$kernel" | tr -c 'a-zA-Z0-9' '_' | tr -s '_' | sed 's/^_*//;s/_*$//')
          output_file="${executable_filename}_${sanitized_kernel_name}_metrics"

          analyze_arguments=(
            analyze
            -p "$workload_path"
            -b
            "${current_kernel_metrics[@]}"
            -n per_kernel
          )
          if [ "$legacy_analyze_output" = true ]; then
            rocprof-compute "${analyze_arguments[@]}" > "${output_file}.txt"
          else
            rocprof-compute "${analyze_arguments[@]}" --output-name "$output_file" --output-format txt > /dev/null
          fi
        done
        cd .. || exit
      fi
    else
      #### Manual Mode ####
      if ! _build_profile_filter_arguments "${_metrics_set[@]}"; then
        exit 1
      fi
      # creating profiling data
      rm -rf "workloads/$executable_filename" # Clear possible previous workloads folder TODO: Can be uncommented when not debugging
      rocprof-compute profile --name "$executable_filename" --no-roof --quiet "${profile_filter_arguments[@]}" -- "$executable" "$args"
      gpu_name=$(ls "workloads/$executable_filename/")
      # get the number of kernels and their respective kernel names in the executable
      analysis_output=$(rocprof-compute analyze -p "workloads/$executable_filename/$gpu_name/" --list-stats)

      # Depending on the ROCm compiler/profiler version, rocprof-compute --list-stats may report an AMDHSA kernel-descriptor symbol whose demangled name ends with"[clone .kd]". 
      # Parse the complete Kernel_Name table cell instead of assuming that the kernel name ends with ")". see https://llvm.org/docs/AMDGPUUsage.html?#amdhsa-kernel-name
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

      rm -rf metrics/workloads
      # Need to move everything into metrics folder because the analyze mode cant save the result in a directory
      # and also cant access parent directories
      mv workloads metrics/workloads
      cd metrics || exit

      # Use different file descriptor (3) for the loop because rocprof-compute consumes/reads stdin resulting in only one kernel analyzed
      exec 3< kernel_list.csv
      # analyze each kernel separately
      while IFS=, read -r -u 3 kernel_number raw_kernel_name; do
        # Remove an optional AMDHSA descriptor suffix from the user-facing name.
        display_kernel_name=$( printf '%s' "$raw_kernel_name" | sed 's/[[:space:]]*\[clone \.kd\][[:space:]]*$//')
        # Replace spaces and special characters with underscores.
        sanitized_kernel_name=$(printf '%s' "$display_kernel_name" | tr -c 'a-zA-Z0-9' '_' | tr -s '_' | sed 's/^_*//;s/_*$//')
        output_file="${executable_filename}_${sanitized_kernel_name}_metrics"

        analyze_arguments=(
          analyze
          -p "workloads/$executable_filename/$gpu_name"
          -b
          "${_metrics_set[@]}"
          -n per_kernel
          -k "$kernel_number"
        )
        if [ "$legacy_analyze_output" = true ]; then
          # rocprofiler-compute 3.0.0 prints the report to stdout.
          rocprof-compute "${analyze_arguments[@]}" > "${output_file}.txt"
        else
          # Newer versions can create and name the text report themselves.
          rocprof-compute "${analyze_arguments[@]}" --output-name "$output_file" --output-format txt > /dev/null
        fi
      done
    fi

    end_metrics=$(date +%s.%N)
    metrics_time=$(awk "BEGIN {print $end_metrics - $start_metrics}")
    exec 3<&- # release file descriptor 3
    cd .. || exit # go back to previous work directory
  else 
    echo "Skipping AMD metrics collection: selected analyses do not require AMD metrics."
    metrics_time=0
  fi


########################################################################
# PC Stall Sampling
########################################################################
  ##### Creating file containing PC sampling data when supported by the gpu TODO Could be moved into metrics collection when pc sampling works without problems #####
  stochastic_gpus=$(rocprofv3 -L | grep -A 7 "GPU\s*:" | grep  "Method\s*:\s*stochastic")
  stochastic_gpus="" # TODO remove when rocprof-compute bug is fixed - doesnt collect data right now and data fields are just empty
  if [ -n "$stochastic_gpus" ]; then
      echo "==== collecting PC sampling data"
    
      start_pcsampling=$(date +%s.%N)
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
      end_pcsampling=$(date +%s.%N)
      pcsampling_time=$(awk "BEGIN {print $end_pcsampling - $start_pcsampling}")
      cd .. || exit
  else
      pcsampling_time=0
      echo "==== no GPU on this system supports hardware-based (stochastic) PC sampling"
  fi
fi

########################################################################
# Merge Analysis
########################################################################
#### Perform analysis on the collected data ####
cd "${gpuscout_dir}"/analysis_amd || exit

args_register_spilling=("${assembly}" "${metrics_dir}" "${livereg_dir}" "${json}" "${gpuscout_output_dir}")
args_restrict=("${assembly}" "${metrics_dir}" "${livereg_dir}" "${json}" "${gpuscout_output_dir}")
args_vectorized_load=("${assembly}" "${metrics_dir}" "${livereg_dir}" "${json}" "${gpuscout_output_dir}")
args_atomic_instruction=("${assembly}" "${metrics_dir}" "${json}" "${gpuscout_output_dir}")
args_wavefront_divergence=("${assembly}" "${metrics_dir}" "${json}" "${gpuscout_output_dir}")
args_shared_memory=("${assembly}" "${metrics_dir}" "${json}" "${gpuscout_output_dir}")
args_datatype_conversion=("${assembly}" "${metrics_dir}" "${json}" "${gpuscout_output_dir}")
args_deadlock_detection=("${assembly}" "${json}" "${gpuscout_output_dir}")

start_analysis=$(date +%s.%N)

if [ "$performance_mode" = false ]; then
  # Time a command (wall clock) and print duration.
  # Usage: timed_run "<label>" <command> [args...]
  timed_run() {
    local analysis="$1"
    local binary="./analysis_${analysis}"
    local array_name="args_${analysis}"

    if [ ! -x "$binary" ]; then
        echo "ERROR: Unknown or unavailable AMD analysis binary: $binary" >&2
        return 1
    fi

    if ! declare -p "$array_name" &>/dev/null; then
        echo "ERROR: No argument array defined for AMD analysis: $analysis" >&2
        return 1
    fi

    local -n current_args="$array_name"

    t0=$(date +%s.%N)
    "$binary" "${current_args[@]}" # Execute the analysis binary with its arguments
    t1=$(date +%s.%N)
    dt=$(awk "BEGIN {print $t1 - $t0}")
    echo "Time for $analysis: ${dt}s"
  }
  for analysis in "${enabled_analyses[@]}"; do
    if ! timed_run "$analysis"; then
            echo "ERROR: AMD analysis failed: $analysis" >&2
            exit 1
        fi
    done
else
  # Use Multi-Threading for faster analysis -> each analysis within its own thread
  analysis_logs_dir="${gpuscout_tmp_dir}/analysis_tmp_outputs"
  mkdir -p "$analysis_logs_dir"
  declare -a names=() pids=() rc=()

  for analysis in "${enabled_analyses[@]}"; do 
    binary="./analysis_${analysis}"
    array_name="args_${analysis}"

    if [ ! -x "$binary" ]; then
        echo "ERROR: Unknown or unavailable AMD analysis binary: $binary" >&2
        exit 1
    fi

    if ! declare -p "$array_name" &>/dev/null; then
        echo "ERROR: No argument array defined for AMD analysis: $analysis" >&2
        exit 1
    fi
  done

  timed_run() {
    local analysis="$1"
    local binary="./analysis_${analysis}"
    local array_name="args_${analysis}"
    local private_log="${analysis_logs_dir}/${analysis}.log"

    local -n current_args="$array_name"

    : > "$private_log" # Clear the private log file
    (
        local t0 t1 dt analysis_rc
        t0=$(date +%s.%N)

        if "$binary" "${current_args[@]}"; then
            analysis_rc=0
        else
            analysis_rc=$?
        fi

        t1=$(date +%s.%N)
        dt=$(awk "BEGIN {print $t1 - $t0}")

        echo "Time for $analysis: ${dt}s" 
        exit "$analysis_rc"
    ) >>"$private_log" 2>&1 &

    names+=("$analysis")
    pids+=("$!")
  }
  
  # Launch all selected analyses in parallel
  for analysis in "${enabled_analyses[@]}"; do
    timed_run "$analysis"
  done
  
  analysis_failed=false
  # Wait and collect exit codes
  for i in "${!pids[@]}"; do
    if wait "${pids[$i]}"; then
      rc[$i]=0
    else
      rc[$i]=$?
    fi
  done

  # Print each analysis output separately and in launch order.
  for i in "${!names[@]}"; do
    cat "${analysis_logs_dir}/${names[$i]}.log"

    if [ "${rc[$i]}" -ne 0 ]; then
      echo \
        "ERROR: AMD analysis '${names[$i]}' failed with exit code ${rc[$i]}." \
        >&2
    fi
  done

  if [ "$analysis_failed" = true ]; then
    echo "ERROR: One or more AMD analyses failed." >&2
    exit 1
  fi
fi
echo "All AMD analyses completed successfully."

# output all used files in JSON format
if [ "$json" = true ]; then

    echo "======================================================================================================"
    echo "==== generating JSON output"
    # ./save_to_json ${gpuscout_output_dir} ${gpuscout_tmp_dir}/result-${run_prefix} ${gpuscout_tmp_dir}/nvdisasm-executable-${executable_filename}-sass.txt ${gpuscout_tmp_dir}/nvdisasm-registers-executable-${executable_filename}-sass.txt ${gpuscout_tmp_dir}/nvdisasm-executable-${executable_filename}-ptx.txt ${gpuscout_tmp_dir}/pcsampling_${executable_filename}.txt
    ./save_to_json ${gpuscout_output_dir} ${gpuscout_tmp_dir}/result-${run_prefix} "${assembly}" "${metrics_dir}" "$livereg_dir"
fi

end_analysis=$(date +%s.%N)
analysis_time=$(awk "BEGIN {print $end_analysis - $start_analysis}")

echo "======================================================================================================"
echo "Time for Static Preparation:     ${static_prep_time}s"
echo "Time for Static Detection:       ${static_detect_time}s"
echo "Time for Live Register Analysis: ${livereg_time}s"
if [ "$dry_run" = false ]; then
    echo "Time for PC Sampling:          ${pcsampling_time}s"
    echo "Time for Metrics Collection:   ${metrics_time}s"
fi
echo "Time for Merging Analysis:     ${analysis_time}s"
echo "Total Time:                    $(awk "BEGIN {print $static_prep_time + $static_detect_time + $livereg_time + $pcsampling_time + $metrics_time + $analysis_time}")s"
echo "======================================================================================================"

cd ..
