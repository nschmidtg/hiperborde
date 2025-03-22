import React, { useState } from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';

const FunctionPlotter = () => {
  const [C, setC] = useState(1);
  const [h, setH] = useState(1);
  
  // Función para calcular los puntos de la gráfica
  const calculatePoints = (C, h) => {
    const points = [];
    for (let x = -4; x <= 4; x += 0.1) {
      const xRounded = Math.round(x * 10) / 10; // Redondear para evitar errores de precisión
      const argument = Math.PI * x / (C * Math.sqrt(h));
      const y = h * Math.pow(Math.sin(argument), 2);
      points.push({ x: xRounded, y: y });
    }
    return points;
  };

  const data = calculatePoints(C, h);
  
  // Encontrar los ceros de la función
  const findZeros = (C, h) => {
    // La función es 0 cuando sin(πx/(C√h)) = 0
    // Esto ocurre cuando πx/(C√h) = nπ, donde n es un entero
    // Por lo tanto, x = nC√h, donde n es un entero
    
    const zeros = [];
    for (let n = -5; n <= 5; n++) {
      if (n === 0) {
        zeros.push(0);
      } else {
        zeros.push(n * C * Math.sqrt(h));
      }
    }
    return zeros.filter(zero => zero >= -4 && zero <= 4);
  };
  
  const zeros = findZeros(C, h);

  return (
    <div style={{ padding: '16px', backgroundColor: 'white', borderRadius: '8px', boxShadow: '0 2px 8px rgba(0,0,0,0.1)' }}>
      <h2 style={{ fontSize: '1.25rem', fontWeight: 'bold', marginBottom: '16px' }}>Gráfica de la función: h·sin²(πx/(C√h))</h2>
      
      <div style={{ marginBottom: '24px', display: 'flex', gap: '16px', flexWrap: 'wrap' }}>
        <div>
          <label style={{ display: 'block', marginBottom: '4px' }}>Valor de C:</label>
          <input 
            type="range" 
            min="0.1" 
            max="5" 
            step="0.1" 
            value={C} 
            onChange={(e) => setC(parseFloat(e.target.value))} 
            style={{ width: '100%' }}
          />
          <div style={{ textAlign: 'center' }}>C = {C}</div>
        </div>
        
        <div>
          <label style={{ display: 'block', marginBottom: '4px' }}>Valor de h:</label>
          <input 
            type="range" 
            min="0.1" 
            max="5" 
            step="0.1" 
            value={h} 
            onChange={(e) => setH(parseFloat(e.target.value))} 
            style={{ width: '100%' }}
          />
          <div style={{ textAlign: 'center' }}>h = {h}</div>
        </div>
      </div>
      
      <div style={{ height: '256px', marginBottom: '24px' }}>
        <ResponsiveContainer width="100%" height="100%">
          <LineChart data={data} margin={{ top: 5, right: 5, left: 5, bottom: 5 }}>
            <CartesianGrid strokeDasharray="3 3" />
            <XAxis 
              dataKey="x" 
              type="number" 
              domain={[-4, 4]} 
              label={{ value: 'x', position: 'insideBottomRight', offset: -5 }} 
            />
            <YAxis 
              domain={[0, h+0.5]} 
              label={{ value: 'f(x)', angle: -90, position: 'insideLeft' }} 
            />
            <Tooltip formatter={(value) => value.toFixed(4)} />
            <Line 
              type="monotone" 
              dataKey="y" 
              stroke="#8884d8" 
              dot={false} 
              isAnimationActive={false} 
            />
          </LineChart>
        </ResponsiveContainer>
      </div>
      
      <div style={{ marginBottom: '16px' }}>
        <h3 style={{ fontWeight: 'bold' }}>Valores para los cuales la función es igual a 0:</h3>
        <p style={{ marginTop: '8px' }}>
          x = n·C·√h, donde n es cualquier entero (0, ±1, ±2, ...)
        </p>
        <p style={{ marginTop: '8px' }}>
          Para los valores actuales (C={C}, h={h}), los ceros en el intervalo [-4, 4] son:
        </p>
        <p style={{ marginTop: '4px', fontFamily: 'monospace' }}>
          {zeros.map(zero => zero.toFixed(2)).join(', ')}
        </p>
      </div>
    </div>
  );
};

export default FunctionPlotter;