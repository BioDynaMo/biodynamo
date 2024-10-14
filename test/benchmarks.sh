#!/bin/bash

# Verifica que el número de iteraciones se haya pasado como argumento
if [ -z "$1" ]; then
  echo "Uso: $0 <numero_de_iteraciones>"
  exit 1
fi

# Número de iteraciones
ITERACIONES=$1

# Filtros a usar
FILTROS=("PyramidalCell0" "PyramidalCell1" "SomaClustering0" "SomaClustering1" "TumorConcept0" "TumorConcept1")

# Iterar sobre cada filtro
for FILTRO in "${FILTROS[@]}"; do
  # Archivo CSV para cada filtro
  CSV_FILE="${FILTRO}_tiempos.csv"
  
  # Agregar encabezado al archivo CSV
  echo "Startup (ns),Run (ns)" > "$CSV_FILE"
  
  # Iterar la cantidad de veces especificada
  for ((i=0; i<$ITERACIONES; i++)); do
    # Ejecutar el comando y filtrar los tiempos
    ${BDMSYS}/bin/biodynamo-benchmark --benchmark_filter="$FILTRO" --benchmark_out=/dev/null | \
    grep -oP 'Startup: \d+ ns|Run: \d+ ns' | \
    awk '{gsub(/[^0-9]/,"",$2); print $2}' | \
    paste -sd, - >> "$CSV_FILE"
  done
done

