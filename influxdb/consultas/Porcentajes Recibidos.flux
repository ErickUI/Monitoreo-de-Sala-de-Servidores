from(bucket: "sala_servidores")
  |> range(start: -15m)
  |> filter(fn: (r) => r._measurement == "lecturas_sala" and r._field == "perdidos")
  |> group()
  |> reduce(
      identity: {recibidos: 0.0, perdidos: 0.0},
      fn: (r, accumulator) => ({
        recibidos: accumulator.recibidos + 1.0,
        perdidos: accumulator.perdidos + r._value
      })
  )
  |> map(fn: (r) => ({
      _time: now(),
      _value: if r.recibidos + r.perdidos > 0.0 then 100.0 * r.recibidos / (r.recibidos + r.perdidos) else 100.0
  }))
