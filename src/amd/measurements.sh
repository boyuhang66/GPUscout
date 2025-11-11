#!/bin/bash

cd "${gpuscout_dir}"/analysis_amd || exit

assembly="${gpuscout_tmp_dir}/${executable_filename}.s"
metrics_dir="${gpuscout_tmp_dir}/metrics"

./analysis_register_spilling  "${assembly}" "${metrics_dir}" "${json}" "${gpuscout_output_dir}"

./analysis_restrict "${assembly}" "${metrics_dir}" "${json}" "${gpuscout_output_dir}"

./analysis_vectorized_load "${assembly}" "${metrics_dir}" "${json}" "${gpuscout_output_dir}"

./analysis_atomic_instruction "${assembly}" "${metrics_dir}" "${json}" "${gpuscout_output_dir}"

./analysis_wavefront_divergence "${assembly}" "${metrics_dir}" "${json}" "${gpuscout_output_dir}"

./analysis_shared_memory "${assembly}" "${metrics_dir}" "${json}" "${gpuscout_output_dir}"

./analysis_datatype_conversion "${assembly}" "${metrics_dir}" "${json}" "${gpuscout_output_dir}"

./analysis_deadlock_detection "${assembly}" "${json}" "${gpuscout_output_dir}"

# output all used files in JSON format
if [ "$json" = true ]; then

    echo "======================================================================================================"
    echo "==== generating JSON output"
    # TODO
    echo "==== TODO"
    # ./save_to_json ${gpuscout_output_dir} ${gpuscout_tmp_dir}/result-${run_prefix} ${gpuscout_tmp_dir}/nvdisasm-executable-${executable_filename}-sass.txt ${gpuscout_tmp_dir}/nvdisasm-registers-executable-${executable_filename}-sass.txt ${gpuscout_tmp_dir}/nvdisasm-executable-${executable_filename}-ptx.txt ${gpuscout_tmp_dir}/pcsampling_${executable_filename}.txt
fi

cd ..
