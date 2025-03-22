import React, { useState } from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';

const FunctionPlotter = () => {
  const [C, setC] = useState(1);
  const [h, setH] = useState(1);
  
  // Función para calcular los puntos de la gráfica
  const calculatePoints = (C, h) => {
    const points = [];
    const end = C * Math.sqrt(h); // Valor de x para n=1
    const step = end / 50; // 50 puntos para tener una gráfica suave
    
    for (let x = 0; x <= end; x += step) {
      const xRounded = Math.round(x * 100) / 100; // Redondear para evitar errores de precisión
      const argument = Math.PI * x / (C * Math.sqrt(h));
      const y = h * Math.pow(Math.sin(argument), 2);
      points.push({ x: xRounded, y: y });
    }
    return points;
  };

  const data = calculatePoints(C, h);
  const maxX = C * Math.sqrt(h);
  
  return (
    <div style={{ padding: '16px', backgroundColor: 'white', borderRadius: '8px', boxShadow: '0 2px 8px rgba(0,0,0,0.1)' }}>
      <h2 style={{ fontSize: '1.25rem', fontWeight: 'bold', marginBottom: '16px' }}>Un ciclo de la función: h·sin²(πx/(C√h))</h2>
      
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
          <LineChart data={data} margin={{ top: 5, right: 20, left: 20, bottom: 5 }}>
            <CartesianGrid strokeDasharray="3 3" />
            <XAxis 
              dataKey="x" 
              type="number" 
              domain={[0, maxX]} 
              ticks={[0, maxX/4, maxX/2, 3*maxX/4, maxX]}
              tickFormatter={(value) => value.toFixed(2)}
              label={{ value: 'x', position: 'insideBottomRight', offset: -5 }} 
            />
            <YAxis 
              domain={[0, h+0.1]} 
              label={{ value: 'f(x)', angle: -90, position: 'insideLeft' }} 
            />
            <Tooltip formatter={(value) => value.toFixed(4)} labelFormatter={(value) => `x: ${value}`} />
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
        <h3 style={{ fontWeight: 'bold' }}>Análisis del ciclo:</h3>
        <p style={{ marginTop: '8px' }}>
          Mostrando un ciclo completo desde x = 0 hasta x = C·√h = {maxX.toFixed(3)}
        </p>
        <p style={{ marginTop: '8px' }}>
          Ceros de la función en este intervalo:
        </p>
        <ul style={{ marginTop: '4px', paddingLeft: '20px' }}>
          <li>x = 0</li>
          <li>x = C·√h = {maxX.toFixed(3)}</li>
        </ul>
        <p style={{ marginTop: '8px' }}>
          Valor máximo de la función: h = {h} (ocurre en x = C·√h/2 = {(maxX/2).toFixed(3)})
        </p>
      </div>
    </div>
  );
};

export default FunctionPlotter;