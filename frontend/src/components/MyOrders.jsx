import { useEffect, useState } from "react"

function MyOrders({ currentUser }) {
const [orders, setOrders] = useState([])
const [loading, setLoading] = useState(true)
const [error, setError] = useState("")

const loadOrders = async () => {
try {
setLoading(true)
setError("")

  const token = localStorage.getItem("authToken")

  if (!token) {
    throw new Error("Please login to view your orders")
  }

  if (!currentUser?.id) {
    throw new Error("User information not available")
  }

  const response = await fetch(
    `http://localhost:18080/api/orders?userId=${currentUser.id}`,
    {
      method: "GET",
      headers: {
        Authorization: `Bearer ${token}`,
      },
    }
  )

  const data = await response.json()

  if (!response.ok) {
    throw new Error(
      data.message || "Failed to load orders"
    )
  }

  setOrders(data.orders || [])
} catch (error) {
  setError(error.message)
} finally {
  setLoading(false)
}

}

useEffect(() => {
loadOrders()
}, [currentUser])

const getStatusClass = (status) => {
switch (status) {
case "PLACED":
return "bg-blue-100 text-blue-700"

  case "CONFIRMED":
    return "bg-indigo-100 text-indigo-700"

  case "PREPARING":
    return "bg-yellow-100 text-yellow-700"

  case "READY":
    return "bg-orange-100 text-orange-700"

  case "OUT_FOR_DELIVERY":
    return "bg-purple-100 text-purple-700"

  case "DELIVERED":
    return "bg-green-100 text-green-700"

  case "CANCELLED":
    return "bg-red-100 text-red-700"

  default:
    return "bg-gray-100 text-gray-700"
}

}

if (loading) {
return ( <section className="max-w-6xl mx-auto px-6 py-12"> <div className="bg-white rounded-2xl shadow-md p-10 text-center"> <p className="text-gray-500 text-lg">
Loading your orders... </p> </div> </section>
)
}

if (error) {
return ( <section className="max-w-6xl mx-auto px-6 py-12"> <div className="bg-red-50 text-red-700 rounded-2xl p-6 text-center"> <p className="font-semibold">
{error} </p>

      <button
        onClick={loadOrders}
        className="mt-4 bg-orange-600 text-white px-5 py-2 rounded-lg hover:bg-orange-700"
      >
        Try Again
      </button>
    </div>
  </section>
)

}

return ( <section className="max-w-6xl mx-auto px-6 py-10">

  <div className="flex flex-col md:flex-row md:items-center md:justify-between gap-4 mb-8">

    <div>
      <h1 className="text-4xl font-bold text-gray-800">
        My Orders
      </h1>

      <p className="text-gray-500 mt-2">
        View your previous and current orders.
      </p>
    </div>

    <button
      onClick={loadOrders}
      className="border border-orange-600 text-orange-600 px-5 py-2 rounded-lg hover:bg-orange-50"
    >
      🔄 Refresh
    </button>

  </div>

  {orders.length === 0 ? (
    <div className="bg-white rounded-2xl shadow-md p-12 text-center">

      <div className="text-6xl mb-5">
        📦
      </div>

      <h2 className="text-2xl font-bold text-gray-800">
        No Orders Yet
      </h2>

      <p className="text-gray-500 mt-2">
        Your orders will appear here after you place your first order.
      </p>

    </div>
  ) : (
    <div className="space-y-6">

      {orders.map((order) => (
        <div
          key={order.id}
          className="bg-white rounded-2xl shadow-md p-6"
        >

          <div className="flex flex-col md:flex-row md:items-center md:justify-between gap-4 mb-5">

            <div>
              <h2 className="text-xl font-bold text-gray-800">
                Order #{order.id}
              </h2>

              <p className="text-gray-500 text-sm mt-1">
                {order.paymentMethod}
              </p>
            </div>

            <span
              className={`px-4 py-2 rounded-full font-semibold text-sm ${getStatusClass(
                order.status
              )}`}
            >
              {order.status}
            </span>

          </div>

          <div className="border-t border-b py-4">

            <h3 className="font-semibold text-gray-700 mb-3">
              Items
            </h3>

            <div className="space-y-3">

              {order.items?.map((item, index) => (
                <div
                  key={`${item.id}-${index}`}
                  className="flex justify-between items-center"
                >

                  <div>
                    <p className="font-medium text-gray-800">
                      {item.name}
                    </p>

                    <p className="text-sm text-gray-500">
                      ₹{item.price} × {item.quantity}
                    </p>
                  </div>

                  <p className="font-semibold text-gray-800">
                    ₹{item.price * item.quantity}
                  </p>

                </div>
              ))}

            </div>

          </div>

          <div className="grid grid-cols-1 md:grid-cols-2 gap-4 mt-5">

            <div>
              <p className="text-sm text-gray-500">
                Delivery Address
              </p>

              <p className="font-medium text-gray-800 mt-1">
                {order.address}
              </p>
            </div>

            <div className="md:text-right">

              <p className="text-sm text-gray-500">
                Total Amount
              </p>

              <p className="text-2xl font-bold text-orange-600 mt-1">
                ₹{order.total}
              </p>

            </div>

          </div>

        </div>
      ))}

    </div>
  )}

</section>

)
}

export default MyOrders
