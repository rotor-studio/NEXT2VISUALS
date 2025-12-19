# NEXT2VISUALS – Resumen de iteraciones

- Integración NDI: recepción de máscara NDI con autoselección de fuente, persistencia en `ndi_source.json`, toggle de visibilidad (`M`) y colisiones con umbral ajustable.
- Cascada de partículas GPU: ping-pong FBO float para pos/vel, shaders de update/render, gravedad y ruido ajustables, rebote opcional sobre máscara, tamaño dinámico, formas punto/cuadrado.
- Trails: FBO de arrastre con factor de desvanecimiento (`trail fade`) y render fresco de la cascada encima para asegurar visibilidad.
- Salida NDI: emite el frame compuesto como “NEXT2VISUALS Output” (sin GUI ni overlays).
- GUI (`G`): sliders para gravedad, ruido, threshold, tamaño, densidad de partículas, bias superior, amortiguación de rebote, reducción de tamaño, alpha de máscara, trail fade, colisión/invertir máscara, mostrar máscara, usar cuadrados; persiste en `bin/data/settings.xml`.
- Controles rápidos: `C` activa/desactiva colisiones, `M` muestra/oculta máscara, `G` muestra GUI; partículas siempre visibles aunque la máscara esté oculta.

Repositorio: git@github.com:rotor-studio/NEXT2VISUALS.git
